#ifndef k4_LCIO_CONVERTER_H
#define k4_LCIO_CONVERTER_H

#include "EVENT/LCEvent.h"
#include "EVENT/LCCollection.h"
#include "podio/CollectionBase.h"
#include "podio/CollectionIDTable.h"

#include <string>
#include <vector>
#include <map>

class k4LCIOConverter
{
public:
    k4LCIOConverter(podio::CollectionIDTable *table);
    ~k4LCIOConverter();

    void set(EVENT::LCEvent *evt);

    podio::CollectionBase *getCollection(const std::string &name, bool add_to_map=true);

private:
    // convertion functions
    podio::CollectionBase *cnvEventHeader();
    podio::CollectionBase *cnvMCParticleCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvSimTrackerHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvTPCHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvTrackerHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvTrackerHitPlaneCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvTrackCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvSimCalorimeterHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvRawCalorimeterHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvCalorimeterHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvParticleIDCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvClusterCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvVertexCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvReconstructedParticleCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvAssociationCollection(EVENT::LCCollection *src);

    // get the index of a LCIO object in a LCCollection, return -1 if not found
    int getIndexOf(EVENT::LCObject *lcObj, EVENT::LCCollection *lcCol);

    // get the edm4hep object that corresponding to a LCIO object
    template <typename edm4hep_t,typename edm4hep_col_t, typename LCIO_t>
    edm4hep_t getCorresponding(const std::string &type, LCIO_t *rvar);

    typedef podio::CollectionBase *(k4LCIOConverter::*cnvfunc)(EVENT::LCCollection *);
    // key: collection type, value: corresponding convertion function
    std::map<std::string, cnvfunc> m_cnv;

    // data holders
    EVENT::LCEvent *m_evt;
    std::map<std::string, EVENT::LCCollection *> m_name2src;
    std::map<std::string, podio::CollectionBase *> m_name2dest;

    //Collection Name to Collection, for collections that are maybe not needed
    std::map<std::string, podio::CollectionBase *> m_name2dest_tmp;

    typedef std::pair<EVENT::LCCollection *, podio::CollectionBase *> CollectionPair;
    // key: collection type, value: corresponding collection pairs
    std::map<std::string, std::vector<CollectionPair>> m_type2cols;

    /// ...
    podio::CollectionIDTable *m_table;
};

template <typename edm4hep_t, typename edm4hep_col_t, typename LCIO_t>
edm4hep_t k4LCIOConverter::getCorresponding(const std::string &type, LCIO_t *robj)
{
    // loop all collections in the same type
    for (auto &colpair : m_type2cols[type])
    {
        int idx = getIndexOf(robj, colpair.first);
        if (idx >= 0)
        {
            auto lcol = dynamic_cast<edm4hep_col_t *>(colpair.second);
            return lcol->at(idx);
        }
    }
    return nullptr;  //crash the application if the corresponding obj is not found
}

#endif
