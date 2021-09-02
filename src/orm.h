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


        explicit Field(std::string N) :
            Name(std::move(N))
        {
            Type = FT_TEXT;
        }

        Field(std::string N, int S) :
            Name(std::move(N)), Size(S)
        {
            Type = FT_TEXT;
        }

        Field(std::string N, int S, bool I):
            Name(std::move(N)), Size(S), Index(I)
        {
            Type = FT_TEXT;
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

    enum SqlComparison { EQ, NEQ, LT, LTE, GT, GTE };
    enum SqlBinaryOp { AND, OR };

    inline std::string to_string(SqlComparison O) {
        switch(O) {
            case EQ: return "=";
            case NEQ: return "!=";
            case GT: return ">";
            case GTE: return ">=";
            case LT: return "<";
            case LTE: return "<=";
        }
        return "=";
    }

    template<typename T> struct SqlOp {
        const char * F;
        SqlComparison   O;
        const T V;
        SqlOp(const char *FieldName, SqlComparison Op, const T Value) : F(FieldName), O(Op), V(Value) {
        };
    };

    inline std::string MkSqlOp( const SqlOp<std::string> & T) {
        return std::string{"("} + T.F + to_string(T.O) + "'" + T.V + "')" ;
    }

    inline std::string MkSqlOp( const SqlOp<const char *> & T) {
        return std::string{"("} + T.F + to_string(T.O) + "'" + T.V + "')" ;
    }

    inline std::string MkSqlOp( const SqlOp<uint64_t> & T) {
        return std::string{"("} + T.F + to_string(T.O) + std::to_string(T.V) + ")" ;
    }

    inline std::string MkSqlOp( const SqlOp<int> & T) {
        return std::string{"("} + T.F + to_string(T.O) + std::to_string(T.V) + ")" ;
    }

    template <typename... Others> std::string MkSqlOp( const SqlOp<std::string> & T, SqlBinaryOp BOP, Others... More) {
        switch(BOP) {
            case AND:
                return  MkSqlOp(T) + " and " + MkSqlOp(More...);
            case OR:
                return MkSqlOp(T) + " or " + MkSqlOp(More...);
        }
        return "";
    }

    template <typename... Others> std::string MkSqlOp( const SqlOp<const char *> & T, SqlBinaryOp BOP, Others... More) {
        switch(BOP) {
            case AND:
                return MkSqlOp(T) + " and " + MkSqlOp(More...);
                case OR:
                    return MkSqlOp(T) + " or " + MkSqlOp(More...);
        }
        return "";
    }

    template <typename... Others> std::string MkSqlOp( const SqlOp<uint64_t> & T, SqlBinaryOp BOP, Others... More) {
        switch(BOP) {
            case AND:
                return MkSqlOp(T) + " and " + MkSqlOp(More...);
                case OR:
                    return MkSqlOp(T) + " or " + MkSqlOp(More...);
        }
        return "";
    }

    template <typename... Others> std::string MkSqlOp( const SqlOp<int> & T, SqlBinaryOp BOP, Others... More) {
        switch(BOP) {
            case AND:
                return MkSqlOp(T) + " and " + MkSqlOp(More...);
                case OR:
                    return MkSqlOp(T) + " or " + MkSqlOp(More...);
        }
        return "";
    }

    inline std::string MkSqlOp( const std::string & T) {
        return T ;
    }

    template <typename... Others> std::string MkSqlOp( const std::string &P1 , SqlBinaryOp BOP, Others... More) {
        switch(BOP) {
            case AND:
                return std::string{"("} + P1 + ") and " + MkSqlOp(More...);
                case OR:
                    return std::string{"("} + P1 + ") or " + MkSqlOp(More...);
        }
        return "";
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



/*
        std::string MakeWhere( const std::string &S, CompareOperations Op, const std::string &V) {
            std::string R;
            assert( FieldNames_.find(S) != FieldNames_.end() );
            R = S + OpsToString[Op] + "'" + Escape(V) + "'" ;
            return R;
        }

        [[maybe_unused]] std::string MakeWhere( const std::string &S, CompareOperations & Op, uint64_t V) {
            std::string R;

            assert( FieldNames_.find(S) != FieldNames_.end() );
            R = S + OpsToString[Op] + std::to_string(V) ;
            return R;
        }
*/

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

                // std::cout << "GetRecords: " << St << std::endl;

                Select  << St ,
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

        inline bool AddUser( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::users, FieldName, ParentUUID, ChildUUID, true);
        }

        inline bool DelUser( const char *FieldName, std::string & ParentUUID, std::string & ChildUUID) {
            return ManipulateVectorMember(&RecordType::users, FieldName, ParentUUID, ChildUUID, false);
        }

        inline bool AddInUse(const char *FieldName, std::string & ParentUUID, const std::string & Prefix, const std::string & ChildUUID) {
            std::string FakeUUID{ Prefix + ":" + ChildUUID};
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, FakeUUID, true);
        }

        inline bool DeleteInUse(const char *FieldName, std::string & ParentUUID, const std::string & Prefix, const std::string & ChildUUID) {
            std::string FakeUUID{ Prefix + ":" + ChildUUID};
            return ManipulateVectorMember(&RecordType::inUse,FieldName, ParentUUID, FakeUUID, false);
        }

        [[nodiscard]] inline std::string ComputeRange(uint64_t From, uint64_t HowMany) {
            if(From<1) From=1;
            switch(Type) {
                case ORM::sqlite:
                    return " LIMIT " + std::to_string(From-1) + ", " + std::to_string(HowMany) +  " ";
                case ORM::postgresql:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
                case ORM::mysql:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
                default:
                    return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From-1) + " ";
            }
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