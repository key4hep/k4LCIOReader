#ifndef K4_LCIO_CONVERTER_H
#define K4_LCIO_CONVERTER_H

#include "EVENT/LCEvent.h"
#include "EVENT/LCCollection.h"
#include "podio/CollectionBase.h"
#include "podio/CollectionIDTable.h"

#include <string>
#include <vector>
#include <map>

class K4LCIOConverter
{
public:
    K4LCIOConverter(podio::CollectionIDTable *table);
    ~K4LCIOConverter();

    void set(EVENT::LCEvent *evt);

    podio::CollectionBase *getCollection(const std::string &name);

private:
    // convertion functions
    podio::CollectionBase *cnvEventHeader();
    podio::CollectionBase *cnvMCParticleCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvSimTrackerHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvTPCHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvTrackerHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvTrackCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvSimCalorimeterHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvRawCalorimeterHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvCalorimeterHitCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvParticleIDCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvClusterCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvVertexCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvReconstructedParticleCollection(EVENT::LCCollection *src);
    podio::CollectionBase *cnvAssociationCollection(EVENT::LCCollection *src);

    // get the edm4hep object that corresponding to a LCIO object
    template <typename edm4hep_t,typename edm4hep_col_t, typename LCIO_t>
    edm4hep_t getCorresponding(const std::string &type, LCIO_t *rvar);

    typedef podio::CollectionBase *(K4LCIOConverter::*cnvfunc)(EVENT::LCCollection *);
    // key: collection type, value: corresponding convertion function
    std::map<std::string, cnvfunc> m_cnv;

    // data holders
    EVENT::LCEvent *m_evt;
    std::map<std::string, EVENT::LCCollection *> m_name2src;
    std::map<std::string, podio::CollectionBase *> m_name2dest;

    typedef std::pair<EVENT::LCCollection *, podio::CollectionBase *> CollectionPair;
    // key: collection type, value: corresponding collection pairs
    std::map<std::string, std::vector<CollectionPair>> m_type2cols;

    /// ...
    podio::CollectionIDTable *m_table;
};

template <typename edm4hep_t, typename edm4hep_col_t, typename LCIO_t>
edm4hep_t K4LCIOConverter::getCorresponding(const std::string &type, LCIO_t *robj)
{
    // loop all collections in the same type
    for (auto &colpair : m_type2cols[type])
    {
        auto rcol = colpair.first;
        for (unsigned j = 0, M = rcol->getNumberOfElements(); j < M; ++j)
        {
            if (robj == rcol->getElementAt(j))
            {
                auto lcol = dynamic_cast<edm4hep_col_t *>(colpair.second);
                return lcol->at(j);
            }
        }
    }
    return edm4hep_t();
}

#endif