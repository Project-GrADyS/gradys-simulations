//
// Generated file, do not edit! Created by nedtool 5.6 from communication/messages/network/DadcaMessage.msg.
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
#include "DadcaMessage_m.h"

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

namespace inet {

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
    omnetpp::cEnum *e = omnetpp::cEnum::find("inet::DadcaMessageType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("inet::DadcaMessageType"));
    e->insert(HEARTBEAT, "HEARTBEAT");
    e->insert(PAIR_REQUEST, "PAIR_REQUEST");
    e->insert(PAIR_CONFIRM, "PAIR_CONFIRM");
    e->insert(BEARER, "BEARER");
)

Register_Class(DadcaMessage)

DadcaMessage::DadcaMessage() : ::inet::FieldsChunk()
{
    this->setChunkLength(B(34));

}

DadcaMessage::DadcaMessage(const DadcaMessage& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

DadcaMessage::~DadcaMessage()
{
}

DadcaMessage& DadcaMessage::operator=(const DadcaMessage& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void DadcaMessage::copy(const DadcaMessage& other)
{
    this->sourceID = other.sourceID;
    this->destinationID = other.destinationID;
    this->nextWaypointID = other.nextWaypointID;
    this->lastWaypointID = other.lastWaypointID;
    this->dataLength = other.dataLength;
    this->leftNeighbours = other.leftNeighbours;
    this->rightNeighbours = other.rightNeighbours;
    this->reversed = other.reversed;
    this->messageType = other.messageType;
}

void DadcaMessage::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->sourceID);
    doParsimPacking(b,this->destinationID);
    doParsimPacking(b,this->nextWaypointID);
    doParsimPacking(b,this->lastWaypointID);
    doParsimPacking(b,this->dataLength);
    doParsimPacking(b,this->leftNeighbours);
    doParsimPacking(b,this->rightNeighbours);
    doParsimPacking(b,this->reversed);
    doParsimPacking(b,this->messageType);
}

void DadcaMessage::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->sourceID);
    doParsimUnpacking(b,this->destinationID);
    doParsimUnpacking(b,this->nextWaypointID);
    doParsimUnpacking(b,this->lastWaypointID);
    doParsimUnpacking(b,this->dataLength);
    doParsimUnpacking(b,this->leftNeighbours);
    doParsimUnpacking(b,this->rightNeighbours);
    doParsimUnpacking(b,this->reversed);
    doParsimUnpacking(b,this->messageType);
}

int DadcaMessage::getSourceID() const
{
    return this->sourceID;
}

void DadcaMessage::setSourceID(int sourceID)
{
    handleChange();
    this->sourceID = sourceID;
}

int DadcaMessage::getDestinationID() const
{
    return this->destinationID;
}

void DadcaMessage::setDestinationID(int destinationID)
{
    handleChange();
    this->destinationID = destinationID;
}

int DadcaMessage::getNextWaypointID() const
{
    return this->nextWaypointID;
}

void DadcaMessage::setNextWaypointID(int nextWaypointID)
{
    handleChange();
    this->nextWaypointID = nextWaypointID;
}

int DadcaMessage::getLastWaypointID() const
{
    return this->lastWaypointID;
}

void DadcaMessage::setLastWaypointID(int lastWaypointID)
{
    handleChange();
    this->lastWaypointID = lastWaypointID;
}

int DadcaMessage::getDataLength() const
{
    return this->dataLength;
}

void DadcaMessage::setDataLength(int dataLength)
{
    handleChange();
    this->dataLength = dataLength;
}

int DadcaMessage::getLeftNeighbours() const
{
    return this->leftNeighbours;
}

void DadcaMessage::setLeftNeighbours(int leftNeighbours)
{
    handleChange();
    this->leftNeighbours = leftNeighbours;
}

int DadcaMessage::getRightNeighbours() const
{
    return this->rightNeighbours;
}

void DadcaMessage::setRightNeighbours(int rightNeighbours)
{
    handleChange();
    this->rightNeighbours = rightNeighbours;
}

bool DadcaMessage::getReversed() const
{
    return this->reversed;
}

void DadcaMessage::setReversed(bool reversed)
{
    handleChange();
    this->reversed = reversed;
}

inet::DadcaMessageType DadcaMessage::getMessageType() const
{
    return this->messageType;
}

void DadcaMessage::setMessageType(inet::DadcaMessageType messageType)
{
    handleChange();
    this->messageType = messageType;
}

class DadcaMessageDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_sourceID,
        FIELD_destinationID,
        FIELD_nextWaypointID,
        FIELD_lastWaypointID,
        FIELD_dataLength,
        FIELD_leftNeighbours,
        FIELD_rightNeighbours,
        FIELD_reversed,
        FIELD_messageType,
    };
  public:
    DadcaMessageDescriptor();
    virtual ~DadcaMessageDescriptor();

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

Register_ClassDescriptor(DadcaMessageDescriptor)

DadcaMessageDescriptor::DadcaMessageDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::DadcaMessage)), "inet::FieldsChunk")
{
    propertynames = nullptr;
}

DadcaMessageDescriptor::~DadcaMessageDescriptor()
{
    delete[] propertynames;
}

bool DadcaMessageDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<DadcaMessage *>(obj)!=nullptr;
}

const char **DadcaMessageDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *DadcaMessageDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int DadcaMessageDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 9+basedesc->getFieldCount() : 9;
}

unsigned int DadcaMessageDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_sourceID
        FD_ISEDITABLE,    // FIELD_destinationID
        FD_ISEDITABLE,    // FIELD_nextWaypointID
        FD_ISEDITABLE,    // FIELD_lastWaypointID
        FD_ISEDITABLE,    // FIELD_dataLength
        FD_ISEDITABLE,    // FIELD_leftNeighbours
        FD_ISEDITABLE,    // FIELD_rightNeighbours
        FD_ISEDITABLE,    // FIELD_reversed
        FD_ISEDITABLE,    // FIELD_messageType
    };
    return (field >= 0 && field < 9) ? fieldTypeFlags[field] : 0;
}

const char *DadcaMessageDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "sourceID",
        "destinationID",
        "nextWaypointID",
        "lastWaypointID",
        "dataLength",
        "leftNeighbours",
        "rightNeighbours",
        "reversed",
        "messageType",
    };
    return (field >= 0 && field < 9) ? fieldNames[field] : nullptr;
}

int DadcaMessageDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 's' && strcmp(fieldName, "sourceID") == 0) return base+0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "destinationID") == 0) return base+1;
    if (fieldName[0] == 'n' && strcmp(fieldName, "nextWaypointID") == 0) return base+2;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lastWaypointID") == 0) return base+3;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dataLength") == 0) return base+4;
    if (fieldName[0] == 'l' && strcmp(fieldName, "leftNeighbours") == 0) return base+5;
    if (fieldName[0] == 'r' && strcmp(fieldName, "rightNeighbours") == 0) return base+6;
    if (fieldName[0] == 'r' && strcmp(fieldName, "reversed") == 0) return base+7;
    if (fieldName[0] == 'm' && strcmp(fieldName, "messageType") == 0) return base+8;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *DadcaMessageDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_sourceID
        "int",    // FIELD_destinationID
        "int",    // FIELD_nextWaypointID
        "int",    // FIELD_lastWaypointID
        "int",    // FIELD_dataLength
        "int",    // FIELD_leftNeighbours
        "int",    // FIELD_rightNeighbours
        "bool",    // FIELD_reversed
        "inet::DadcaMessageType",    // FIELD_messageType
    };
    return (field >= 0 && field < 9) ? fieldTypeStrings[field] : nullptr;
}

const char **DadcaMessageDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_messageType: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *DadcaMessageDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_messageType:
            if (!strcmp(propertyname, "enum")) return "inet::DadcaMessageType";
            return nullptr;
        default: return nullptr;
    }
}

int DadcaMessageDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    DadcaMessage *pp = (DadcaMessage *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *DadcaMessageDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    DadcaMessage *pp = (DadcaMessage *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string DadcaMessageDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    DadcaMessage *pp = (DadcaMessage *)object; (void)pp;
    switch (field) {
        case FIELD_sourceID: return long2string(pp->getSourceID());
        case FIELD_destinationID: return long2string(pp->getDestinationID());
        case FIELD_nextWaypointID: return long2string(pp->getNextWaypointID());
        case FIELD_lastWaypointID: return long2string(pp->getLastWaypointID());
        case FIELD_dataLength: return long2string(pp->getDataLength());
        case FIELD_leftNeighbours: return long2string(pp->getLeftNeighbours());
        case FIELD_rightNeighbours: return long2string(pp->getRightNeighbours());
        case FIELD_reversed: return bool2string(pp->getReversed());
        case FIELD_messageType: return enum2string(pp->getMessageType(), "inet::DadcaMessageType");
        default: return "";
    }
}

bool DadcaMessageDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    DadcaMessage *pp = (DadcaMessage *)object; (void)pp;
    switch (field) {
        case FIELD_sourceID: pp->setSourceID(string2long(value)); return true;
        case FIELD_destinationID: pp->setDestinationID(string2long(value)); return true;
        case FIELD_nextWaypointID: pp->setNextWaypointID(string2long(value)); return true;
        case FIELD_lastWaypointID: pp->setLastWaypointID(string2long(value)); return true;
        case FIELD_dataLength: pp->setDataLength(string2long(value)); return true;
        case FIELD_leftNeighbours: pp->setLeftNeighbours(string2long(value)); return true;
        case FIELD_rightNeighbours: pp->setRightNeighbours(string2long(value)); return true;
        case FIELD_reversed: pp->setReversed(string2bool(value)); return true;
        case FIELD_messageType: pp->setMessageType((inet::DadcaMessageType)string2enum(value, "inet::DadcaMessageType")); return true;
        default: return false;
    }
}

const char *DadcaMessageDescriptor::getFieldStructName(int field) const
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

void *DadcaMessageDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    DadcaMessage *pp = (DadcaMessage *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

} // namespace inet

