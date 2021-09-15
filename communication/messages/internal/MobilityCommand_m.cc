//
// Generated file, do not edit! Created by nedtool 5.6 from communication/messages/internal/MobilityCommand.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "MobilityCommand_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}

namespace projeto {

// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("projeto::MobilityCommandType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("projeto::MobilityCommandType"));
    e->insert(REVERSE, "REVERSE");
    e->insert(GOTO_WAYPOINT, "GOTO_WAYPOINT");
    e->insert(GOTO_COORDS, "GOTO_COORDS");
)

Register_Class(MobilityCommand)

MobilityCommand::MobilityCommand(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

MobilityCommand::MobilityCommand(const MobilityCommand& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

MobilityCommand::~MobilityCommand()
{
}

MobilityCommand& MobilityCommand::operator=(const MobilityCommand& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void MobilityCommand::copy(const MobilityCommand& other)
{
    this->commandType = other.commandType;
    this->param1 = other.param1;
    this->param2 = other.param2;
    this->param3 = other.param3;
    this->param4 = other.param4;
    this->param5 = other.param5;
}

void MobilityCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->commandType);
    doParsimPacking(b,this->param1);
    doParsimPacking(b,this->param2);
    doParsimPacking(b,this->param3);
    doParsimPacking(b,this->param4);
    doParsimPacking(b,this->param5);
}

void MobilityCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->commandType);
    doParsimUnpacking(b,this->param1);
    doParsimUnpacking(b,this->param2);
    doParsimUnpacking(b,this->param3);
    doParsimUnpacking(b,this->param4);
    doParsimUnpacking(b,this->param5);
}

projeto::MobilityCommandType MobilityCommand::getCommandType() const
{
    return this->commandType;
}

void MobilityCommand::setCommandType(projeto::MobilityCommandType commandType)
{
    this->commandType = commandType;
}

double MobilityCommand::getParam1() const
{
    return this->param1;
}

void MobilityCommand::setParam1(double param1)
{
    this->param1 = param1;
}

double MobilityCommand::getParam2() const
{
    return this->param2;
}

void MobilityCommand::setParam2(double param2)
{
    this->param2 = param2;
}

double MobilityCommand::getParam3() const
{
    return this->param3;
}

void MobilityCommand::setParam3(double param3)
{
    this->param3 = param3;
}

double MobilityCommand::getParam4() const
{
    return this->param4;
}

void MobilityCommand::setParam4(double param4)
{
    this->param4 = param4;
}

double MobilityCommand::getParam5() const
{
    return this->param5;
}

void MobilityCommand::setParam5(double param5)
{
    this->param5 = param5;
}

class MobilityCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_commandType,
        FIELD_param1,
        FIELD_param2,
        FIELD_param3,
        FIELD_param4,
        FIELD_param5,
    };
  public:
    MobilityCommandDescriptor();
    virtual ~MobilityCommandDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(MobilityCommandDescriptor)

MobilityCommandDescriptor::MobilityCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(projeto::MobilityCommand)), "omnetpp::cMessage")
{
    propertynames = nullptr;
}

MobilityCommandDescriptor::~MobilityCommandDescriptor()
{
    delete[] propertynames;
}

bool MobilityCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<MobilityCommand *>(obj)!=nullptr;
}

const char **MobilityCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *MobilityCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int MobilityCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount() : 6;
}

unsigned int MobilityCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_commandType
        FD_ISEDITABLE,    // FIELD_param1
        FD_ISEDITABLE,    // FIELD_param2
        FD_ISEDITABLE,    // FIELD_param3
        FD_ISEDITABLE,    // FIELD_param4
        FD_ISEDITABLE,    // FIELD_param5
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *MobilityCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "commandType",
        "param1",
        "param2",
        "param3",
        "param4",
        "param5",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int MobilityCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'c' && strcmp(fieldName, "commandType") == 0) return base+0;
    if (fieldName[0] == 'p' && strcmp(fieldName, "param1") == 0) return base+1;
    if (fieldName[0] == 'p' && strcmp(fieldName, "param2") == 0) return base+2;
    if (fieldName[0] == 'p' && strcmp(fieldName, "param3") == 0) return base+3;
    if (fieldName[0] == 'p' && strcmp(fieldName, "param4") == 0) return base+4;
    if (fieldName[0] == 'p' && strcmp(fieldName, "param5") == 0) return base+5;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *MobilityCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "projeto::MobilityCommandType",    // FIELD_commandType
        "double",    // FIELD_param1
        "double",    // FIELD_param2
        "double",    // FIELD_param3
        "double",    // FIELD_param4
        "double",    // FIELD_param5
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **MobilityCommandDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_commandType: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *MobilityCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_commandType:
            if (!strcmp(propertyname, "enum")) return "projeto::MobilityCommandType";
            return nullptr;
        default: return nullptr;
    }
}

int MobilityCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    MobilityCommand *pp = (MobilityCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *MobilityCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    MobilityCommand *pp = (MobilityCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string MobilityCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    MobilityCommand *pp = (MobilityCommand *)object; (void)pp;
    switch (field) {
        case FIELD_commandType: return enum2string(pp->getCommandType(), "projeto::MobilityCommandType");
        case FIELD_param1: return double2string(pp->getParam1());
        case FIELD_param2: return double2string(pp->getParam2());
        case FIELD_param3: return double2string(pp->getParam3());
        case FIELD_param4: return double2string(pp->getParam4());
        case FIELD_param5: return double2string(pp->getParam5());
        default: return "";
    }
}

bool MobilityCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    MobilityCommand *pp = (MobilityCommand *)object; (void)pp;
    switch (field) {
        case FIELD_commandType: pp->setCommandType((projeto::MobilityCommandType)string2enum(value, "projeto::MobilityCommandType")); return true;
        case FIELD_param1: pp->setParam1(string2double(value)); return true;
        case FIELD_param2: pp->setParam2(string2double(value)); return true;
        case FIELD_param3: pp->setParam3(string2double(value)); return true;
        case FIELD_param4: pp->setParam4(string2double(value)); return true;
        case FIELD_param5: pp->setParam5(string2double(value)); return true;
        default: return false;
    }
}

const char *MobilityCommandDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *MobilityCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    MobilityCommand *pp = (MobilityCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

} // namespace projeto

