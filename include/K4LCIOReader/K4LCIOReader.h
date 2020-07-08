#ifndef K4_LCIOREADER_H
#define K4_LCIOREADER_H

#include "podio/IReader.h"
#include "podio/CollectionIDTable.h"
#include <map>

/// forward declarations
// in LCIO
namespace IO
{
class LCReader;
}
// in podio
namespace podio
{
class EventStore;
}
// others
class K4LCIOConverter;

class K4LCIOReader : public podio::IReader
{
    friend podio::EventStore;

public:
    K4LCIOReader();
    ~K4LCIOReader();

    void open(const std::string &filename);
    void open(const std::vector<std::string> &filenames);
    void close();

    void setReadCollectionNames(const std::vector<std::string> &colnames);

    /// Prepare LCCollections requested
    bool readNextEvent();

    /// get an edm4hep collection
    template <typename T>
    T *getCollection(const std::string &name);

    /// Read CollectionIDTable from ROOT file
    podio::CollectionIDTable *getCollectionIDTable() override final { return m_table; }

    /// Returns number of entries in the file
    unsigned getEntries() const { return m_entries; }

    /// Preparing to read next event
    //void endOfEvent();

    /// Preparing to read a given event
    //void goToEvent(unsigned evnum);

    /// Check if TFile is valid
    virtual bool isValid() const override final;

    /// pure virtual methods that from parent class
    virtual podio::GenericParameters* readEventMetaData();
    virtual std::map<int, podio::GenericParameters>* readCollectionMetaData();
    virtual std::map<int, podio::GenericParameters>* readRunMetaData();

private:
    /// Implementation for collection reading
    podio::CollectionBase *readCollection(const std::string &name) override final;

private:
    unsigned m_entries;

    //LCReader in LCIO
    IO::LCReader *m_reader;

    //the converter
    K4LCIOConverter *m_converter;

    ///...
    podio::CollectionIDTable *m_table;
};

template <typename T>
T *K4LCIOReader::getCollection(const std::string &name)
{
    auto p = dynamic_cast<T *>(readCollection(name));
    if (p == nullptr)
    {
        p = new T();
        p->setID(m_table->add(name));
    }
    return p;
}

#endif
