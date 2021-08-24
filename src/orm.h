//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef __OPENWIFI_ORM_H__
#define __OPENWIFI_ORM_H__

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <array>

#include "Poco/Tuple.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Logger.h"

namespace ORM {
    enum DBType {
        sqlite,
        postgresql,
        mysql
    };

    enum FieldType {
        FT_INT,
        FT_BIGINT,
        FT_TEXT,
        FT_VARCHAR,
        FT_BLOB
    };

    enum Indextype {
        ASC,
        DESC
    };

    struct Field {
        std::string Name;
        FieldType   Type;
        int         Size=0;
        bool        Index=false;


        Field(std::string & N) {
            Name = N;
            Type = FT_TEXT;
        }

        Field(std::string N, int S) {
            Name = N;
            Type = FT_TEXT;
            Size = S;
        }

        Field(std::string N, int S, bool I) {
            Name = N;
            Type = FT_TEXT;
            Size = S;
            Index = I;
        }
    };
    typedef std::vector<Field>  FieldVec;

    struct IndexEntry {
        std::string     FieldName;
        Indextype   Type;
    };
    typedef std::vector<IndexEntry>     IndexEntryVec;

    struct Index {
        std::string         Name;
        IndexEntryVec   Entries;
    };
    typedef std::vector<Index>      IndexVec;

    inline std::string FieldTypeToChar(DBType Type, FieldType T, int Size=0) {
        switch(T) {
            case FT_INT:    return "INT";
            case FT_BIGINT: return "BIGINT";
            case FT_TEXT:   return "TEXT";
            case FT_VARCHAR:
                if(Size)
                    return std::string("VARCHAR(") + std::to_string(Size) + std::string(")");
                else
                    return "TEXT";
                case FT_BLOB:
                    if(Type==DBType::mysql)
                        return "LONGBLOB";
                    else if(Type==DBType::postgresql)
                        return "BYTEA";
                    else if(Type==DBType::sqlite)
                        return "BLOB";
                    default:
                        assert(false);
                        return "";
        }
        assert(false);
        return "";
    }

    enum CompareOperations {
        EQUAL = 0 ,
        LESS,
        LESS_OR_EQUAL,
        GREATER,
        GREATER_OR_EQUAL,
        NOT_EQUAL
    };

    static std::vector<std::string> OpsToString{ " = " ,  " < " , " <= " , " > " , " >= " , " != "};
    inline std::string to_string(ORM::CompareOperations O) {
        switch(O) {
            case EQUAL: return "=";
            case LESS: return "<";
            case LESS_OR_EQUAL: return "<=";
            case GREATER: return ">";
            case GREATER_OR_EQUAL: return ">=";
            case NOT_EQUAL: return "!=";
        }
    }

    inline std::string to_string(uint64_t V) {
        return std::to_string(V);
    }

    inline std::string to_string(int V) {
        return std::to_string(V);
    }

    inline std::string to_string(bool V) {
        return std::to_string(V);
    }

    inline std::string to_string(const std::string &S) {
        return S;
    }

    inline std::string to_string(const char * S) {
        return S;
    }

    template <typename ValueType> struct Comparison {
        std::string     Name;
        ValueType       Value;
        ORM::CompareOperations  Operation;
        Comparison(const std::string & N, ORM::CompareOperations Op, ValueType & V) :
            Name(N), Operation(Op), Value(V) {}

        std::string OP() {
            return Name + " " + to_string(Operation) + " " + to_string(Value);
        }
    };

    template <typename RecordTuple, typename RecordType> class DB {
    public:
        DB( DBType dbtype,
            const char *TableName,
            const FieldVec & Fields,
            const IndexVec & Indexes,
            Poco::Data::SessionPool & Pool,
                Poco::Logger &L,
                const char *Prefix):
                Type(dbtype),
                DBName(TableName),
                Pool_(Pool),
                Logger_(L),
                Prefix_(Prefix)
        {
            bool first = true;
            int  Place=0;

            assert( RecordTuple::length == Fields.size());

            for(const auto &i:Fields) {

                FieldNames_[i.Name] = Place;
                if(!first) {
                    CreateFields_ += ", ";
                    SelectFields_ += ", ";
                    UpdateFields_ += ", ";
                    SelectList_ += ", ";
                } else {
                    SelectList_ += "(";
                }

                CreateFields_ += i.Name + " " + FieldTypeToChar(Type, i.Type,i.Size) + (i.Index ? " unique primary key" : "");
                SelectFields_ += i.Name ;
                UpdateFields_ += i.Name + "=?";
                SelectList_ += "?";
                first = false;
                Place++;
            }
            SelectList_ += ")";

            if(!Indexes.empty()) {
                if(Type==sqlite || Type==postgresql) {
                    for(const auto &j:Indexes) {
                        std::string IndexLine;

                        IndexLine = std::string("CREATE INDEX IF NOT EXISTS ") + j.Name + std::string(" ON ") + DBName + " (";
                        bool first_entry=true;
                        for(const auto &k:j.Entries) {
                            assert(FieldNames_.find(k.FieldName) != FieldNames_.end());
                            if(!first_entry) {
                                IndexLine += " , ";
                            }
                            first_entry = false;
                            IndexLine += k.FieldName + std::string(" ") + std::string(k.Type == Indextype::ASC ? "ASC" : "DESC") ;
                        }
                        IndexLine += " );";
                        IndexCreation += IndexLine;
                    }
                } else if(Type==mysql) {
                    bool firstIndex = true;
                    std::string IndexLine;
                    for(const auto &j:Indexes) {
                        if(!firstIndex)
                            IndexLine += ", ";
                        firstIndex = false;
                        IndexLine += " INDEX " + j.Name + " ( " ;
                        bool first_entry=true;
                        for(const auto &k:j.Entries) {
                            assert(FieldNames_.find(k.FieldName) != FieldNames_.end());
                            if(!first_entry) {
                                IndexLine += " ,";
                            }
                            first_entry = false;
                            IndexLine += k.FieldName + std::string(k.Type == Indextype::ASC ? " ASC" : " DESC");
                        }
                        IndexLine += " ) ";
                    }
                    IndexCreation = IndexLine;
                }
            }
        }

        [[nodiscard]] const std::string & CreateFields() const { return CreateFields_; };
        [[nodiscard]] const std::string & SelectFields() const { return SelectFields_; };
        [[nodiscard]] const std::string & SelectList() const { return SelectList_; };
        [[nodiscard]] const std::string & UpdateFields() const { return UpdateFields_; };

        static std::string Escape(const std::string &S) {
            std::string R;

            for(const auto &i:S)
                if(i=='\'')
                    R += "''";
                else
                    R += i;
                return R;
        }

        std::string MakeWhere( const std::string &S, CompareOperations Op, const std::string &V) {
            std::string R;

            assert( FieldNames_.find(S) != FieldNames_.end() );

            R = S + OpsToString[Op] + "\"" + Escape(V) + "\"" ;

            return R;
        }

        std::string MakeWhere( const std::string &S, CompareOperations & Op, uint64_t &V) {
            std::string R;

            assert( FieldNames_.find(S) != FieldNames_.end() );

            R = S + OpsToString[Op] + std::to_string(V) ;

            return R;
        }

        inline bool  Create() {
            std::string S;

            if(Type==mysql) {
                if(IndexCreation.empty())
                    S = "create table if not exists " + DBName +" ( " + CreateFields_ + " )" ;
                else
                    S = "create table if not exists " + DBName +" ( " + CreateFields_ + " ), " + IndexCreation + " )";
            } else if (Type==postgresql || Type==sqlite) {
                S = "create table if not exists " + DBName + " ( " + CreateFields_ + " ); " + IndexCreation ;
            }

            // std::cout << "CREATE-DB: " << S << std::endl;

            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   CreateStatement(Session);

                CreateStatement << S;
                CreateStatement.execute();
                return true;
            } catch (const Poco::Exception &E) {
                std::cout << "Exception while creating DB: " << E.name() << std::endl;
            }
            return false;
        }

        [[nodiscard]] std::string ConvertParams(const std::string & S) const {
            std::string R;

            R.reserve(S.size()*2+1);

            if(Type==postgresql) {
                auto Idx=1;
                for(auto const & i:S)
                {
                    if(i=='?') {
                        R += '$';
                        R.append(std::to_string(Idx++));
                    } else {
                        R += i;
                    }
                }
            } else {
                R = S;
            }
            return R;
        }

        void Convert( RecordTuple &in , RecordType &out);
        void Convert( RecordType &in , RecordTuple &out);

        inline const std::string & Prefix() { return Prefix_; };

        bool CreateRecord( RecordType & R) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Insert(Session);

                RecordTuple RT;
                Convert(R, RT);
                std::string St = "insert into  " + DBName + " ( " + SelectFields_ + " ) values " + SelectList_;
                Insert  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(RT);
                Insert.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template<typename T> bool GetRecord( const char * FieldName, T Value,  RecordType & R) {
            try {

                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple             RT;

                std::string St = "select " + SelectFields_ + " from " + DBName + " where " + FieldName + "=?" ;

                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(Value);
                if(Select.execute()==1) {
                    Convert(RT,R);
                    return true;
                }
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        typedef std::vector<std::string> StringVec;

        template <  typename T,
                typename T0, typename T1> bool GR(const char *FieldName, T & R,T0 &V0, T1 &V1) {
            try {

                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple RT;

                std::string St = "select " + SelectFields_ + " from " + DBName
                                + " where " + FieldName[0] + "=? and " + FieldName[1] + "=?"  ;
                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(V0),
                    Poco::Data::Keywords::use(V1);

                if(Select.execute()==1) {
                    Convert(RT,R);
                    return true;
                }
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        typedef std::vector<RecordTuple> RecordList;

        bool GetRecords( uint64_t Offset, uint64_t HowMany, std::vector<RecordType> & Records, const std::string & Where = "") {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordList RL;
                std::string St = "select " + SelectFields_ + " from " + DBName +
                        (Where.empty() ? "" : " where " + Where) +
                        ComputeRange(Offset, HowMany) ;
                std::string St2 = Where.empty() ? ConvertParams(St) : (ConvertParams(St) + " where " + Where) ;

                // std::cout << "GetRecords: " << St2 << std::endl;

                Select  << St2 ,
                    Poco::Data::Keywords::into(RL);

                if(Select.execute()>0) {
                    for(auto &i:RL) {
                        RecordType  R;
                        Convert(i, R);
                        Records.template emplace_back(R);
                    }
                    return true;
                }
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool UpdateRecord( const char *FieldName, T & Value,  RecordType & R) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Update(Session);

                RecordTuple RT;

                Convert(R, RT);

                std::string St = "update " + DBName + " set " + UpdateFields_ + " where " + FieldName + "=?" ;
                Update  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(RT),
                    Poco::Data::Keywords::use(Value);
                Update.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool DeleteRecord( const char *FieldName, T Value) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Delete(Session);

                std::string St = "delete from " + DBName + " where " + FieldName + "=?" ;
                Delete  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(Value);
                Delete.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool DeleteRecords( const std::string & WhereClause ) {
            try {
                assert( !WhereClause.empty());
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Delete(Session);

                std::string St = "delete from " + DBName + " where " + WhereClause;
                Delete  << St;
                Delete.execute();
                return true;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        bool Exists(const char *FieldName, std::string & Value) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                RecordType  R;
                if(GetRecord(FieldName,Value,R))
                    return true;
                return false;
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        uint64_t Count( const std::string & Where="" ) {
            try {
                uint64_t Cnt=0;

                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);

                std::string st{"SELECT COUNT(*) FROM " + DBName + " " + (Where.empty() ? "" : (" where " + Where)) };

                Select << st ,
                    Poco::Data::Keywords::into(Cnt);
                Select.execute();

                return Cnt;

            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return 0;
        }

        template <typename X> bool ManipulateVectorMember( X T, const char *FieldName, std::string & ParentUUID, std::string & ChildUUID, bool Add) {
            try {
                assert( FieldNames_.find(FieldName) != FieldNames_.end() );

                RecordType R;
                if(GetRecord(FieldName, ParentUUID, R)) {
                    auto it = std::lower_bound((R.*T).begin(),(R.*T).end(),ChildUUID);
                    if(Add) {
                        if(it!=(R.*T).end() && *it == ChildUUID)
                            return false;
                        (R.*T).insert(it, ChildUUID);
                    } else {
                        if(it!=(R.*T).end() && *it == ChildUUID)
                            (R.*T).erase(it);
                        else
                            return false;
                    }
                    UpdateRecord(FieldName, ParentUUID, R);
                    return true;
                }
            } catch (const Poco::Exception &E) {
                Logger_.log(E);
            }
            return false;
        }

        inline bool AddChild( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::children, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteChild( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::children, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddLocation( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteLocation( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::locations, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddContact( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteContact( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::contacts, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddVenue( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::venues, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteVenue( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::venues, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddDevice( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::devices, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteDevice( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::devices, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddEntity( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::entities, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteEntity( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::entities, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddInUse(const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DeleteInUse(const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, ChildUUID, false);
        }

        [[nodiscard]] inline std::string ComputeRange(uint64_t From, uint64_t HowMany) {
            if(From<1) From=1;
            if(Type==ORM::sqlite) {
                return " LIMIT " + std::to_string(From-1) + ", " + std::to_string(HowMany) +  " ";
            } else if(Type==ORM::postgresql) {
                return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
            } else if(Type==ORM::mysql) {
                return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
            }
            return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
        }

    private:
        DBType                      Type;
        std::string                 DBName;
        std::string                 CreateFields_;
        std::string                 SelectFields_;
        std::string                 SelectList_;
        std::string                 UpdateFields_;
        std::string                 IndexCreation;
        std::map<std::string,int>   FieldNames_;
        Poco::Data::SessionPool     &Pool_;
        Poco::Logger                &Logger_;
        std::string                 Prefix_;
    };
}

#endif