#include "K4LCIOReader/K4LCIOReader.h"
#include "K4LCIOConverter.h"

#include "podio/CollectionIDTable.h"
#include "IO/LCReader.h"
#include "IOIMPL/LCFactory.h"

#include <iostream>

K4LCIOReader::K4LCIOReader()
    : m_entries(0),
      m_reader(nullptr)
{
    m_table = new podio::CollectionIDTable();
    m_converter = new K4LCIOConverter(m_table);
}

K4LCIOReader::~K4LCIOReader()
{
    //terminate
    close();
    delete m_converter;
}

void K4LCIOReader::open(const std::string &filename)
{
    open(std::vector<std::string>{filename});
}

void K4LCIOReader::open(const std::vector<std::string> &filenames)
{
    if (isValid())
    {
        close();
    }

    m_reader = IOIMPL::LCFactory::getInstance()->createLCReader();

    for (const auto &file : filenames)
    {
        m_reader->open(file);
        m_entries += m_reader->getNumberOfEvents();
        m_reader->close();
    }

    m_reader->open(filenames);
}

void K4LCIOReader::close()
{
    if (m_reader)
    {
        m_reader->close();
        delete m_reader;
        m_reader = nullptr;
        m_entries = 0;
    }
}

void K4LCIOReader::setReadCollectionNames(const std::vector<std::string> &colnames)
{
    if (isValid())
    {
        m_reader->setReadCollectionNames(colnames);
    }
}

bool K4LCIOReader::readNextEvent()
{
    auto evt = m_reader->readNextEvent();
    if (!evt)
    {
        this->close();
        return false;
    }

    m_converter->set(evt);

    return true;
}

bool K4LCIOReader::isValid() const
{
    return (m_reader != nullptr);
}

podio::GenericParameters* K4LCIOReader::readEventMetaData()
{
    //not implemented yet
    return nullptr;
}

std::map<int, podio::GenericParameters>* K4LCIOReader::readCollectionMetaData()
{
    //not implemented yet
    return nullptr;
}

std::map<int, podio::GenericParameters>* K4LCIOReader::readRunMetaData()
{
    //not implemented yet
    return nullptr;
}

podio::CollectionBase *K4LCIOReader::readCollection(const std::string &name)
{
    return m_converter->getCollection(name);
}
