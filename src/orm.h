
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
        EQUAL,
        LESS,
        LESS_OR_EQUAL,
        GREATER,
        GREATER_OR_EQUAL,
        NOT_EQUAL
    };

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
                Poco::Logger &L):
                Type(dbtype),
                DBName(TableName),
                Pool_(Pool),
                Logger_(L)
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

        std::string ConvertParams(const std::string & S) const {
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

        virtual void Convert( RecordTuple &in , RecordType &out);
        virtual void Convert( RecordType &in , RecordTuple &out);

        bool CreateRecord( RecordType & R) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Insert(Session);

                RecordTuple RT;
                ConvertParams(R, RT);

                std::string St = "insert into  " + DBName + " ( " + SelectFields_ + " ) values " + SelectList_;
                Insert  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(RT),
                Insert.execute();
                return true;
            } catch (const Poco::Exception &E) {
                // Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool GetRecord( const std::string &FieldName, T Value,  RecordType & R) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple RT;

                std::string St = "select " + SelectFields_ + " from " + DBName + " where " + FieldName + "=?" ;
                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(Value);
                Select.execute();

                Convert(RT,R);
                return true;
            } catch (const Poco::Exception &E) {
                // Logger_.log(E);
            }
            return false;
        }

        typedef std::vector<std::string> StringVec;

        template <  typename T,
                    typename T0, typename T1> bool GR(const StringVec &FieldName, T & R,T0 &V0, T1 &V1) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordTuple RT;

                std::string St = "select " + SelectFields_ + " from " + DBName
                                + " where " + FieldName[0] + "=? and " + FieldName[1] + "=?"  ;
                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RT),
                    Poco::Data::Keywords::use(V0),
                    Poco::Data::Keywords::use(V1);
                Select.execute();

                Convert(RT,R);
                return true;
            } catch (const Poco::Exception &E) {
                // Logger_.log(E);
            }
            return false;
        }

        typedef std::vector<RecordTuple> RecordList;

        bool GetRecords( uint64_t Offset, uint64_t HowMany, std::vector<RecordType> & Records, std::string Where = "") {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Select(Session);
                RecordList RL;
                std::string St = "select " + SelectFields_ + " from " + DBName + ComputeRange(Offset, HowMany) ;

                Select  << ConvertParams(St) ,
                    Poco::Data::Keywords::into(RL);
                Select.execute();

                for(const auto &i:RL) {
                    RecordType  R;
                    Convert(i, R);
                    Records.template emplace_back(R);
                }
                return true;
            } catch (const Poco::Exception &E) {
                // Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool UpdateRecord( const std::string &FieldName, T & Value,  RecordType & R) {
            try {
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
                // Logger_.log(E);
            }
            return false;
        }

        template <typename T> bool DeleteRecord( const std::string &FieldName, T Value) {
            try {
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Delete(Session);

                std::string St = "delete from " + DBName + " where " + FieldName + "=?" ;
                Delete  << ConvertParams(St) ,
                    Poco::Data::Keywords::use(Value);
                Delete.execute();
                return true;
            } catch (const Poco::Exception &E) {
                // Logger_.log(E);
            }
            return false;
        }

        bool DeleteRecords( const std::string & WhereClause ) {
            try {
                assert( !WhereClause.empty());
                Poco::Data::Session     Session = Pool_.get();
                Poco::Data::Statement   Delete(Session);

                std::string St = "delete from " + DBName + " " + WhereClause;
                    Delete  << ConvertParams(St);
                Delete.execute();
                return true;
            } catch (const Poco::Exception &E) {
                // Logger_.log(E);
            }
            return false;
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
    };
}

/*
int main(int, char**)
{
    auto SQLiteConn_ = std::make_unique<Poco::Data::SQLite::Connector>();
    SQLiteConn_->registerConnector();
    auto Pool_ = std::make_unique<Poco::Data::SessionPool>(SQLiteConn_->name(), "test.db", 4, 64, 60);

    ORM::FieldVec    DB1Fields{     ORM::Field{"id",40, true},
                                    ORM::Field{"name", ORM::FT_INT} };
    ORM::IndexVec    Indexes{
        { std::string("name_index"), ORM::IndexEntryVec{ {std::string("name"), ORM::Indextype::ASC} } } };

    ORM::DB<DB1_record, Rec1>   DB1(ORM::sqlite ,
                                    "tab1",
                                    ORM::FieldVec{
                                                    ORM::Field{"id",40, true},
                                                    ORM::Field{"name", ORM::FT_INT} },
                                    ORM::IndexVec{{ std::string("name_index"), ORM::IndexEntryVec{ {std::string("name"), ORM::Indextype::ASC} } }},
                                    *Pool_);

    std::cout << DB1.CreateFields() << std::endl;
    std::cout << DB1.SelectFields() << std::endl;
    std::cout << DB1.SelectList() << std::endl;
    std::cout << DB1.UpdateFields() << std::endl;

    std::cout << DB1.Create() << std::endl;

    User    U1{ 25, "steph"};

    auto SS = &User::Name;

    std::cout << (typeid( User::Name ) == typeid( std::string )) << std::endl;
    std::cout << "Name: " << U1.*SS << std::endl;

    auto RR = ORM::Escape("I'm a \"cool\" dude");
    std::cout << "RR: " << RR << std::endl;

    return 0;
}

 */

#endif