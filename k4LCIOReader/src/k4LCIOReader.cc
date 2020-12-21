#include "k4LCIOReader/k4LCIOReader.h"
#include "k4LCIOConverter.h"

#include "podio/CollectionIDTable.h"
#include "podio/GenericParameters.h"
#include "IO/LCReader.h"
#include "IOIMPL/LCFactory.h"

#include <iostream>

k4LCIOReader::k4LCIOReader()
    : m_entries(0),
      m_reader(nullptr)
{
    m_table = new podio::CollectionIDTable();
    m_converter = new k4LCIOConverter(m_table);
}

k4LCIOReader::~k4LCIOReader()
{
    //terminate
    closeFiles();
    delete m_converter;
}

void k4LCIOReader::openFile(const std::string &filename)
{
    openFiles(std::vector<std::string>{filename});
}

void k4LCIOReader::openFiles(const std::vector<std::string> &filenames)
{
    if (isValid())
    {
        closeFile();
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

void k4LCIOReader::closeFile()
{
    if (m_reader)
    {
        m_reader->close();
        delete m_reader;
        m_reader = nullptr;
        m_entries = 0;
    }
}

void k4LCIOReader::closeFiles()
{
    this->closeFile();
}


void k4LCIOReader::setReadCollectionNames(const std::vector<std::string> &colnames)
{
    if (isValid())
    {
        m_reader->setReadCollectionNames(colnames);
    }
}

bool k4LCIOReader::readNextEvent()
{
    auto evt = m_reader->readNextEvent();
    if (!evt)
    {
        this->closeFiles();
        return false;
    }

    m_converter->set(evt);

    return true;
}

bool k4LCIOReader::isValid() const
{
    return (m_reader != nullptr);
}

podio::GenericParameters* k4LCIOReader::readEventMetaData()
{
    //not implemented yet
    return nullptr;
}

std::map<int, podio::GenericParameters>* k4LCIOReader::readCollectionMetaData()
{
    //not implemented yet
    return nullptr;
}

std::map<int, podio::GenericParameters>* k4LCIOReader::readRunMetaData()
{
    //not implemented yet
    return nullptr;
}

void k4LCIOReader::endOfEvent()
{
    //not implemented yet
}

podio::CollectionBase *k4LCIOReader::readCollection(const std::string &name)
{
    return m_converter->getCollection(name);
}
