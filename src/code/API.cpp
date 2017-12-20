#include "API.h"
#include "DataStruct.h"

bool API::isInt(string str) {
    std::istringstream iss(str);
    int f;
    iss >> noskipws >> f;
    return iss.eof() && !iss.fail();
}

bool API::isFloat(string str) {
    std::istringstream iss(str);
    float f;
    iss >> noskipws >> f;
    return iss.eof() && !iss.fail();
}

API::API() {

}

void API::tableDrop(string table_name) {
    auto &rm = RecordManager::instance();
    auto &cm = CatalogManager::instance();
    
    if (!cm.hasTable(table_name)) {
        cout << "No such Table\n";
        return;
    }
    
    Table& t = cm.getTable(table_name);
    
    for (auto &walker: t.attrs) {
        if (walker.has_index) {
            indexDrop(walker.index_name);
        }
    }

    rm.dropTableFile(table_name);
    cm.dropTable(table_name);
    cout << "Drop table " << table_name << " successfully!\n";
}

void API::indexDrop(string index_name) {

    auto &cm = CatalogManager::instance();
    auto &im = IndexManager::instance();
    if (!cm.hasIndex(index_name)) {
        cout << "No such Index\n";
        return;
    }
    // get indextype!
    auto it = cm.getIndex(index_name);
    Attribute &att = getAttribute(it.first, it.second);
    
    im.dropIndex(index_name, att.type);
    cm.dropIndex(index_name);
    cout << "Drop index " << index_name << " successfully\n";
}

void API::tableCreate(string table_name, vector<Attribute>* attribute_vector) {
    auto &rm = RecordManager::instance();
    auto &cm = CatalogManager::instance();
   
    if(cm.hasTable(table_name)) {
        cout << "There is a table " << table_name << " already" << endl;
        return;
    }
    
    rm.createTableFile(table_name);
    
    cm.createTable(table_name, *attribute_vector);
        
    cout << "Create table " << table_name << " successfully\n";
    
    for (auto &walker: *attribute_vector) {
        if (walker.is_primary_key) {
                indexCreate(walker.index_name, table_name, walker.name);
            break;
        }
    }
}

void API::indexCreate(string index_name, string table_name, string attribute_name) {
    
    auto &cm = CatalogManager::instance();
    auto &im = IndexManager::instance();
    bool hasAttr = false;
    int type = 0, size = 0;
    
    if (cm.hasIndex(index_name)) {
        cout << "There is index " << index_name << " already" << endl;
        return;
    }
    
    if (!cm.hasTable(table_name)) {
        cout << "No such table\n";
        return;
    }
    
    vector<Attribute> attribute_vector = cm.getTable(table_name).attrs;
    
    for (auto &walker: attribute_vector) {
        if (attribute_name == walker.name) {
            if (!walker.is_unique) {
                cout << "This Attribute is not unique!\n";
                return;
            }
            hasAttr = true;
            type = walker.type;
            size = walker.size;
        }
    }
    
    if (!hasAttr) {
        cout << "There is no such attribute in the table\n";
        return;
    }
    
    // insert all the records into index if the index already have some records
    if (im.createIndexFile(index_name)) {
        im.createIndex(index_name, type, size);
        cm.createIndex(index_name, table_name, attribute_name);
        // insert all records of this index into the index
        cout << "Create Index " << index_name << " successfully\n";
    }
    else cout << "Create Index " << index_name << " failed\n";
}

void API::recordShow(string table_name, vector<string>* attribute_name_vector, vector<Condition>* condition_vector) {
    auto &rm = RecordManager::instance();
    auto &cm = CatalogManager::instance();
    auto &im = IndexManager::instance();

    QueryResult res;
    if (!cm.hasTable(table_name)) {
        cout << "No such Table\n";
        return;
    }
    Table& t = cm.getTable(table_name);
    vector<Attribute> attribute_vector = t.attrs;
    vector<string> allAttributeName;
    if (attribute_name_vector == NULL) {
        for (auto &attribute : attribute_vector) {
            allAttributeName.insert(allAttributeName.end(), attribute.name);
        }
        
        attribute_name_vector = &allAttributeName;
    }
    
    for (auto &name: *attribute_name_vector) {
        bool hasAttr = false;
        for (auto &rightAttr: attribute_vector) {
            if (rightAttr.name == name) {
                hasAttr = true;
                break;
            }
        }
        if (!hasAttr) {
            cout << "No such attribute in projection" << endl;
            return;
        }
    }

    if (condition_vector == NULL) {
        res = rm.selectAllRecords(table_name);

    }
    else {
        for (auto &condition: *condition_vector) {
            bool hasAttr = false;
            for (auto &rightAttr: attribute_vector) {
                if (rightAttr.name == condition.attr_name) {
                    hasAttr = true;
                    if (rightAttr.has_index) {
                        im.createIndex(rightAttr.index_name, rightAttr.type, rightAttr.size);
                    }
                    break;
                }
            }
            if (!hasAttr) {
                cout << "No such attribute in condition" << endl;
                return;
            }
        }

        res = rm.selectRecord(table_name, *condition_vector);
    }

    printQueryResults(res, attribute_name_vector);
    
    cout << res.size() << " records selected\n";
}

void API::recordInsert(string table_name, vector<string>* record_content) {
    auto &rm = RecordManager::instance();
    auto &cm = CatalogManager::instance();
    auto &im = IndexManager::instance();

    if (!cm.hasTable(table_name)) {
        cout << "No such table\n";
        return;
    }
    
    Table &t = cm.getTable(table_name);
    
    vector<Attribute> attribute_vector = t.attrs;
    Record rec;
    
    if (attribute_vector.size() != record_content->size()) {
        cout << "Value count does not match!\n";
        return;
    }
    
    int i = 0;
    for (auto &walker: attribute_vector) {
        if (walker.type == INT) {
            if (isInt((*record_content)[i])) {
                Element ele(atoi((*record_content)[i].c_str()));
                if (walker.is_unique && !matchUnique(table_name, walker, ele)) {
                    cout << "Duplicate values\n";
                    return;
                }
            
                rec.push_back(ele);
                if (walker.has_index) {
                    if (im.createIndexFile(walker.index_name))
                    im.createIndex(walker.index_name, walker.type, walker.size);
                }
            }
            else {
                cout << "Insert value type error\n";
                return;
            }
        }
        else if (walker.type == FLOAT) {
            if (isFloat((*record_content)[i])) {
                Element ele((float)atof((*record_content)[i].c_str()));
                if (walker.is_unique && !matchUnique(table_name, walker, ele)) {
                    cout << "Duplicate values\n";
                    return;
                }
                
                rec.push_back(ele);
                if (walker.has_index) {
                    im.createIndex(walker.index_name, walker.type, walker.size);
                }
            }
            else {
                cout << "Insert value type error\n";
                return;
            }
        }
        else if (walker.type == STRING) {
            if ((*record_content)[i].size() <= walker.size) {
                Element ele((*record_content)[i], walker.size);
                if (walker.is_unique && !matchUnique(table_name, walker, ele)) {
                    cout << "Duplicate values\n";
                    return;
                }
                
                rec.push_back(ele);
                if (walker.has_index) {
                    im.createIndex(walker.index_name, walker.type, walker.size);
                }
            }
            else {
                cout << "Insert value is too big\n";
                return;
            }
        }
        i++;
    }
    
    rm.insertRecord(table_name, rec);
    
    cout << "Insert Successfully\n";
}

void API::recordDelete(string table_name) {
    vector<Condition> condition_vector;
    recordDelete(table_name, &condition_vector);
}

void API::recordDelete(string table_name, vector<Condition>* condition_vector) {
    auto &rm = RecordManager::instance();
    auto &cm = CatalogManager::instance();
    auto &im = IndexManager::instance();
    
    // index delete!!

    if (!cm.hasTable(table_name)) {
        cout << "No such table\n";
        return;
    }
  
    Table& t = cm.getTable(table_name);
    vector<Attribute> attribute_vector = t.attrs;
    
    for (auto &condition: *condition_vector) {
        bool hasAttr = false;
        for (auto &rightAttr: attribute_vector) {
            if (rightAttr.name == condition.attr_name) {
                hasAttr = true;
                if (rightAttr.has_index) {
                    im.createIndex(rightAttr.index_name, rightAttr.type, rightAttr.size);
                }
                break;
            }
        }
        if (!hasAttr) {
            cout << "No such attribute in condition" << endl;
            return;
        }
    }
    
    int deleteNum = rm.deleteRecord(table_name, *condition_vector);

    cout << "deleted " << deleteNum << " records\n";
}

Attribute& API::getAttribute(string table_name, string attribute_name) {

    auto &cm = CatalogManager::instance();

    if (!cm.hasTable(table_name)) {
        Attribute *att = new Attribute;
        att->name = "";
        return *att;
    }
    Table& t = cm.getTable(table_name);
    for (auto walker: t.attrs) {
        if (walker.name == attribute_name) {
            Attribute *att = new Attribute(walker.name, walker.type, walker.size, walker.is_unique, walker.has_index, walker.is_primary_key);
            if (att->has_index) att->index_name = walker.index_name;
            return *att;
        }
    }
    Attribute *att = new Attribute;
    att->name = "";
    return *att;
}

void API::printQueryResults(QueryResult res, vector<string>* attribute_name_vector) {
    // to do
    //int *length = (int *)malloc(sizeof(int) * attribute_name_vector->size());
    int length[10005] = {0};
    int i = 0;
    int temp = 0;
    if (res.size() == 0) return;
    for (i = 0; i < (*attribute_name_vector).size(); i++) {
        length[i] = (*attribute_name_vector)[i].size();
    }

    for (auto &walker: res) {
        i = 0;
        for (auto &walker2: walker) {
            if (walker2.type == INT) {
                temp = (int)to_string(walker2.data._int).size();
                length[i] = length[i] > temp ? length[i] : temp;
            }
            else if (walker2.type == FLOAT) {
                length[i] = length[i] > 5 ? length[i] : 5;
            }
            else if (walker2.type == STRING) {
                temp = walker2._string.size();
                length[i] = length[i] > temp ? length[i] : temp;
            }
            i++;
        }
    }
    for (i = 0; i < (*attribute_name_vector).size(); i++) {
        cout << "+-";
        for (int j = 0; j < length[i]; j++) cout << "-";
        cout << "-";
    }
    cout << "+" << endl;
    for (i = 0; i < (*attribute_name_vector).size(); i++) {
        cout << "| ";
        cout << (*attribute_name_vector)[i];
        for (int j = 0; j < length[i] - (*attribute_name_vector)[i].size(); j++) cout << " ";
        cout << " ";
    }
    cout << "|" << endl;
    for (i = 0; i < (*attribute_name_vector).size(); i++) {
        cout << "+-";
        for (int j = 0; j < length[i]; j++) cout << "-";
        cout << "-";
    }
    cout << "+" << endl;
    for (auto &walker: res) {
        i = 0;
        for (auto &walker2: walker) {
            cout << "| ";
            if (walker2.type == INT) {
                cout << walker2.data._int;
                for (int j = 0; j < length[i] - (int)to_string(walker2.data._int).size(); j++) cout << " ";
            }
            else if (walker2.type == FLOAT) {
                cout.setf(ios::showpoint);
                cout << setprecision(4) << walker2.data._float;
            }
            else if (walker2.type == STRING) {
                cout << walker2._string;
                for (int j = 0; j < length[i] - walker2._string.size(); j++) cout << " ";
            }
            cout << " ";
            i++;
        }
        cout << "|" << endl;
    }
    for (i = 0; i < (*attribute_name_vector).size(); i++) {
        cout << "+-";
        for (int j = 0; j < length[i]; j++) cout << "-";
        cout << "-";
    }
    cout << "+" << endl;
    return;
}
//+---------+-------------+--------+--------+--------------+-----------+
//| book_id | name        | price  | number | storage_date | author    |
//+---------+-------------+--------+--------+--------------+-----------+
//| 1       | HarryPotter | 100.00 |      9 | 2017-04-28   | JKRolling |
//| 2       | ThreeBody   | 100.00 |      2 | 2017-05-02   | LiuCixin  |
//| 3       | Database    | 100.00 |      0 | 2017-05-04   | CG        |
//+---------+-------------+--------+--------+--------------+-----------+

bool API::matchUnique(const string table_name, const Attribute att, const Element ele) {
    auto &rm = RecordManager::instance();
    Condition c(att.name, ele, OPERATOR_EQUAL);
    vector<Condition> cond_vec;
    cond_vec.push_back(c);
    QueryResult temp = rm.selectRecord(table_name, cond_vec);
    if (temp.size() > 0) return false;
    return true;
}
