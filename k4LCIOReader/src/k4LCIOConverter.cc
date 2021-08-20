#include "k4LCIOReader/k4LCIOConverter.h"

//LCIO headers
#include "EVENT/MCParticle.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/TPCHit.h"
#include "EVENT/TrackerHit.h"
#include "EVENT/TrackerHitPlane.h"
#include "EVENT/Track.h"
#include "EVENT/SimCalorimeterHit.h"
#include "EVENT/RawCalorimeterHit.h"
#include "EVENT/CalorimeterHit.h"
#include "EVENT/ParticleID.h"
#include "EVENT/Cluster.h"
#include "EVENT/Vertex.h"
#include "EVENT/ReconstructedParticle.h"
#include "EVENT/LCRelation.h"
#include "EVENT/LCParameters.h"

//EDM4hep headers
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TPCHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/TrackerHitPlaneCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/RawCalorimeterHitCollection.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/ParticleIDCollection.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/VertexCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoParticleAssociationCollection.h"

k4LCIOConverter::k4LCIOConverter(podio::CollectionIDTable *table)
    : m_table(table)
{
    m_cnv["MCParticle"] = &k4LCIOConverter::cnvMCParticleCollection;
    m_cnv["SimTrackerHit"] = &k4LCIOConverter::cnvSimTrackerHitCollection;
    m_cnv["TPCHit"] = &k4LCIOConverter::cnvTPCHitCollection;
    m_cnv["TrackerHit"] = &k4LCIOConverter::cnvTrackerHitCollection;
    m_cnv["TrackerHitPlane"] = &k4LCIOConverter::cnvTrackerHitPlaneCollection;
    m_cnv["Track"] = &k4LCIOConverter::cnvTrackCollection;
    m_cnv["SimCalorimeterHit"] = &k4LCIOConverter::cnvSimCalorimeterHitCollection;
    m_cnv["RawCalorimeterHit"] = &k4LCIOConverter::cnvRawCalorimeterHitCollection;
    m_cnv["CalorimeterHit"] = &k4LCIOConverter::cnvCalorimeterHitCollection;
    m_cnv["ParticleID"] = &k4LCIOConverter::cnvParticleIDCollection;
    m_cnv["Cluster"] = &k4LCIOConverter::cnvClusterCollection;
    m_cnv["Vertex"] = &k4LCIOConverter::cnvVertexCollection;
    m_cnv["ReconstructedParticle"] = &k4LCIOConverter::cnvReconstructedParticleCollection;
    m_cnv["LCRelation"] = &k4LCIOConverter::cnvAssociationCollection;
}

k4LCIOConverter::~k4LCIOConverter()
{
}

void k4LCIOConverter::set(EVENT::LCEvent *evt)
{
    m_name2src.clear();
    m_name2dest.clear();
    m_type2cols.clear();

    m_evt = evt;
    for (const auto &colname : *(evt->getCollectionNames()))
    {
        auto pcol = evt->getCollection(colname);
        m_name2src[colname] = pcol;
    }
}

podio::CollectionBase *k4LCIOConverter::getCollection(const std::string &name)
{
    // if already exist
    auto idest = m_name2dest.find(name);
    if (idest != m_name2dest.end())
    {
        return idest->second;
    }

    // in case of EventHeader
    if ( name == "EventHeader")
    {
        auto dest = this->cnvEventHeader();
        dest->setID(m_table->add(name));
        m_name2dest[name] = dest;
        return dest;
    }

    // if not a valid collection
    auto isrc = m_name2src.find(name);
    if (isrc == m_name2src.end())
    {
        m_name2dest[name] = nullptr;
        return nullptr;
    }

    // if not a known type
    EVENT::LCCollection *src = isrc->second;
    auto it = m_cnv.find(src->getTypeName());
    if (it == m_cnv.end())
    {
        std::cout << "Error: do not support convertion of " << name
                  << " with type " << src->getTypeName()
                  << std::endl;
        return nullptr;
    }

    // convert
    podio::CollectionBase *dest = (this->*(it->second))(src);
    dest->setID(m_table->add(name));

    // put result in data holders
    m_name2dest[name] = dest;

    m_type2cols[src->getTypeName()].push_back(std::make_pair(src, dest));

    return dest;
}

static edm4hep::Vector3f Vector3fFrom(const double *v)
{
    return edm4hep::Vector3f(v[0], v[1], v[2]);
}

static edm4hep::Vector3f Vector3fFrom(const EVENT::FloatVec& v)
{
    return edm4hep::Vector3f(v[0], v[1], v[2]);
}

podio::CollectionBase *k4LCIOConverter::cnvEventHeader()
{
    auto dest = new edm4hep::EventHeaderCollection();

    edm4hep::EventHeader lval = dest->create();
    lval.setEventNumber(m_evt->getEventNumber());
    lval.setRunNumber(m_evt->getRunNumber());
    lval.setTimeStamp(m_evt->getTimeStamp());
    lval.setWeight(m_evt->getWeight());

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvMCParticleCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::MCParticleCollection();

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::MCParticle *rval = (EVENT::MCParticle *)src->getElementAt(i);
        edm4hep::MCParticle lval = dest->create();

        lval.setPDG(rval->getPDG());
        lval.setGeneratorStatus(rval->getGeneratorStatus());
        lval.setSimulatorStatus(rval->getSimulatorStatus());
        lval.setCharge(rval->getCharge());
        lval.setTime(rval->getTime());
        lval.setMass(rval->getMass());
        lval.setSpin(edm4hep::Vector3f(rval->getSpin()));
        lval.setColorFlow(edm4hep::Vector2i(rval->getColorFlow()));
        lval.setVertex(edm4hep::Vector3d(rval->getVertex()));
        lval.setEndpoint(edm4hep::Vector3d(rval->getEndpoint()));
        lval.setMomentum(Vector3fFrom(rval->getMomentum()));
        lval.setMomentumAtEndpoint(Vector3fFrom(rval->getMomentumAtEndpoint()));

        // ? suppose there is only one MCParticleCollection in each event
        for (auto rparent : rval->getParents())
        {
            for (unsigned k = 0; k < i; ++k)
            {
                if (src->getElementAt(k) == rparent)
                {
                    // A loop for edm4hep's MCParticleCollection to recover the relationship
                    edm4hep::MCParticle lparent = dest->at(k);
                    lval.addToParents(lparent);
                    lparent.addToDaughters(lval);
                    break;
                }
            }
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvSimTrackerHitCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::SimTrackerHitCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
        if (v.second->getTypeName() == "MCParticle")
            getCollection(v.first);
    });

    // fill the collection
    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::SimTrackerHit *rval = (EVENT::SimTrackerHit *)src->getElementAt(i);
        edm4hep::SimTrackerHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setEDep(rval->getEDep());
        lval.setTime(rval->getTime());
        lval.setPathLength(rval->getPathLength());
        lval.setQuality(rval->getQuality());
        lval.setPosition(rval->getPosition());
        lval.setMomentum(rval->getMomentum());

        // find and set the associated MCParticle
        EVENT::MCParticle *robj = rval->getMCParticle();
        if (!robj)
            continue;
        auto lobj =
            getCorresponding<edm4hep::MCParticle, edm4hep::MCParticleCollection,
                             EVENT::MCParticle>("MCParticle", robj);
        if (lobj.isAvailable())
        {
            lval.setMCParticle(lobj);
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvTPCHitCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::TPCHitCollection();

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::TPCHit *rval = (EVENT::TPCHit *)src->getElementAt(i);
        edm4hep::TPCHit lval = dest->create();

        lval.setCellID(rval->getCellID());
        lval.setTime(rval->getTime());
        lval.setCharge(rval->getCharge());
        lval.setQuality(rval->getQuality());
        for (unsigned j = 0, M = rval->getNRawDataWords(); j < M; j++)
        {
            lval.addToRawDataWords(rval->getRawDataWord(j));
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvTrackerHitCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::TrackerHitCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
        if (v.second->getTypeName() == "TPCHit")
            getCollection(v.first);
    });

    // fill the collection
    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::TrackerHit *rval = (EVENT::TrackerHit *)src->getElementAt(i);
        edm4hep::TrackerHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setType(rval->getType());
        lval.setQuality(rval->getQuality());
        lval.setTime(rval->getTime());
        lval.setEDep(rval->getEDep());
        lval.setEDepError(rval->getEDepError());
        lval.setEdx(rval->getdEdx());
        lval.setPosition(rval->getPosition());
        auto &m = rval->getCovMatrix();
        lval.setCovMatrix({m[0], m[1], m[2], m[3], m[4], m[5]});

        // find and set the associated RawHit ( only TPCHit is considered now )
        auto &robjvec = rval->getRawHits();
        for (auto &robj : robjvec)
        {
            auto lobj =
                getCorresponding<edm4hep::TPCHit, edm4hep::TPCHitCollection,
                                 EVENT::LCObject>("TPCHit", robj);
            if (lobj.isAvailable())
            {
                lval.addToRawHits(lobj.getObjectID());
            }
        }
    }

    return dest;
}


podio::CollectionBase *k4LCIOConverter::cnvTrackerHitPlaneCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::TrackerHitPlaneCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
        if (v.second->getTypeName() == "TPCHit")
            getCollection(v.first);
    });

    // fill the collection
    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::TrackerHitPlane *rval = (EVENT::TrackerHitPlane *)src->getElementAt(i);
        edm4hep::TrackerHitPlane lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setType(rval->getType());
        lval.setQuality(rval->getQuality());
        lval.setTime(rval->getTime());
        lval.setEDep(rval->getEDep());
        lval.setEDepError(rval->getEDepError());
        lval.setEdx(rval->getdEdx());
        lval.setPosition(rval->getPosition());
        lval.setU({rval->getU()[0], rval->getU()[1]});
        lval.setV({rval->getV()[0], rval->getV()[1]});
        lval.setDu(rval->getdU());
        lval.setDv(rval->getdV());
        auto &m = rval->getCovMatrix();
        lval.setCovMatrix({m[0], m[1], m[2], m[3], m[4], m[5]});

        // find and set the associated RawHit ( only TPCHit is considered now )
        auto &robjvec = rval->getRawHits();
        for (auto &robj : robjvec)
        {
            auto lobj =
                getCorresponding<edm4hep::TPCHit, edm4hep::TPCHitCollection,
                                 EVENT::LCObject>("TPCHit", robj);
            if (lobj.isAvailable())
            {
                lval.addToRawHits(lobj.getObjectID());
            }
        }
    }

    return dest;
}



podio::CollectionBase *k4LCIOConverter::cnvTrackCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::TrackCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
        if (v.second->getTypeName() == "TrackerHit")
            getCollection(v.first);
    });

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::Track *rval = (EVENT::Track *)src->getElementAt(i);
        edm4hep::Track lval = dest->create();

        lval.setType(rval->getType());
        lval.setChi2(rval->getChi2());
        lval.setNdf(rval->getNdf());
        lval.setDEdx(rval->getdEdx());
        lval.setDEdxError(rval->getdEdxError());
        lval.setRadiusOfInnermostHit(rval->getRadiusOfInnermostHit());

        // find and set the associated TrackerHits
        auto &rhits = rval->getTrackerHits();
        for (auto &robj : rhits)
        {
            auto lobj =
                getCorresponding<edm4hep::TrackerHit, edm4hep::TrackerHitCollection,
                                 EVENT::TrackerHit>("TrackerHit", robj);
            if (lobj.isAvailable())
            {
                lval.addToTrackerHits(lobj);
            }
        }

        // add Sub-detector numbers
        for (auto v : rval->getSubdetectorHitNumbers())
        {
            lval.addToSubDetectorHitNumbers(v);
        }

        // fill the TrackStates
        auto &rstates = rval->getTrackStates();
        for (auto &robj : rstates)
        {
            std::array<float, 15> _covMatrix;
            const auto& rCM = robj->getCovMatrix();
            for (int i = 0; i < 15; ++i)
            {
                _covMatrix[i] = rCM[i];
            }
            lval.addToTrackStates(edm4hep::TrackState{
                robj->getLocation(),
                robj->getD0(),
                robj->getPhi(),
                robj->getOmega(),
                robj->getZ0(),
                robj->getTanLambda(),
                robj->getReferencePoint(),
                _covMatrix});
        }
    }

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        // find and set the associated tracks
        for (auto &robj : ((EVENT::Track *)src->getElementAt(i))->getTracks())
        {
            int idx = getIndexOf(robj, src);
            auto lobj = idx < 0
                ? getCorresponding<edm4hep::Track, edm4hep::TrackCollection, EVENT::Track>("Track", robj)
                : dest->at(idx);
            if (lobj.isAvailable())
            {
                dest->at(i).addToTracks(lobj);
            }
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvSimCalorimeterHitCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::SimCalorimeterHitCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto& v) {
            if (v.second->getTypeName() == "MCParticle") getCollection(v.first);
            });

    // get the CaloHitContribution collection
    if ( m_name2dest.find("CaloHitContribution_EXT") == m_name2dest.end() ) {
        auto tmpCol = new edm4hep::CaloHitContributionCollection();
        tmpCol->setID(m_table->add("CaloHitContribution_EXT"));
        m_name2dest.insert(std::make_pair("CaloHitContribution_EXT", tmpCol));
    }
    auto caloHitContCol = dynamic_cast<edm4hep::CaloHitContributionCollection*>(m_name2dest["CaloHitContribution_EXT"]);

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::SimCalorimeterHit *rval = (EVENT::SimCalorimeterHit *)src->getElementAt(i);
        edm4hep::SimCalorimeterHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setEnergy(rval->getEnergy());
        lval.setPosition(rval->getPosition());

        // set CaloHitContribution objects
        for (auto j = 0, total = rval->getNMCContributions(); j < total; ++j)
        {
            edm4hep::CaloHitContribution tmpObj = caloHitContCol->create();
            tmpObj.setPDG(rval->getPDGCont(j));
            tmpObj.setEnergy(rval->getEnergyCont(j));
            tmpObj.setTime(rval->getTimeCont(j));
            tmpObj.setStepPosition(rval->getStepPosition(j));
            auto particle =
                getCorresponding<edm4hep::MCParticle, edm4hep::MCParticleCollection,
                                 EVENT::MCParticle>("MCParticle", rval->getParticleCont(j));
            if (particle.isAvailable()) {
                tmpObj.setParticle(particle);
            }
            lval.addToContributions(tmpObj);
        }

        // FIXME: the interface for setting MCParticle is not present in edm4hep now
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvRawCalorimeterHitCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::RawCalorimeterHitCollection();

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::RawCalorimeterHit *rval = (EVENT::RawCalorimeterHit *)src->getElementAt(i);
        edm4hep::RawCalorimeterHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setAmplitude(rval->getAmplitude());
        lval.setTimeStamp(rval->getTimeStamp());
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvCalorimeterHitCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::CalorimeterHitCollection();

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::CalorimeterHit *rval = (EVENT::CalorimeterHit *)src->getElementAt(i);
        edm4hep::CalorimeterHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setEnergy(rval->getEnergy());
        lval.setEnergyError(rval->getEnergyError());
        lval.setPosition(rval->getPosition());
        lval.setTime(rval->getTime());
        lval.setType(rval->getType());

        // FIXME: the interface for setting RawCalorimeterHit is not present in edm4hep now
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvParticleIDCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::ParticleIDCollection();

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::ParticleID *rval = (EVENT::ParticleID *)src->getElementAt(i);
        edm4hep::ParticleID lval = dest->create();

        lval.setType(rval->getType());
        lval.setPDG(rval->getPDG());
        lval.setAlgorithmType(rval->getAlgorithmType());
        lval.setLikelihood(rval->getLikelihood());

        for (auto v : rval->getParameters())
        {
            lval.addToParameters(v);
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvClusterCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::ClusterCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto& v) {
            if (v.second->getTypeName() == "CalorimeterHit") getCollection(v.first);
            });

    // get the ParticleID collection
    if ( m_name2dest.find("ParticleID_EXT") == m_name2dest.end() ) {
        auto tmpCol = new edm4hep::ParticleIDCollection();
        tmpCol->setID(m_table->add("ParticleID_EXT"));
        m_name2dest.insert(std::make_pair("ParticleID_EXT", tmpCol));
    }
    auto particleIdCol = dynamic_cast<edm4hep::ParticleIDCollection*>(m_name2dest["ParticleID_EXT"]);

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::Cluster *rval = (EVENT::Cluster *)src->getElementAt(i);
        edm4hep::Cluster lval = dest->create();

        lval.setEnergy(rval->getEnergy());
        lval.setEnergyError(rval->getEnergyError());
        lval.setITheta(rval->getITheta());
        lval.setPhi(rval->getIPhi());
        lval.setPosition(rval->getPosition());
        auto &m = rval->getPositionError();
        lval.setPositionError({m[0], m[1], m[2], m[3], m[4], m[5]});
        lval.setType(rval->getType());
        lval.setDirectionError(Vector3fFrom(rval->getDirectionError()));

        // find and set the associated CalorimeterHits
        auto &rhits = rval->getCalorimeterHits();
        for (auto &robj : rhits)
        {
            auto lobj =
                getCorresponding<edm4hep::CalorimeterHit, edm4hep::CalorimeterHitCollection,
                                 EVENT::CalorimeterHit>("CalorimeterHit", robj);
            if (lobj.isAvailable())
            {
                lval.addToHits(lobj);
            }
        }

        // generate and set the associated ParticleIDs
        auto &rparIDs = rval->getParticleIDs();
        for (auto &robj : rparIDs)
        {
            edm4hep::ParticleID tmpObj = particleIdCol->create();
            tmpObj.setType( robj->getType() );
            tmpObj.setPDG( robj->getPDG() );
            tmpObj.setAlgorithmType( robj->getAlgorithmType() );
            tmpObj.setLikelihood( robj->getLikelihood() );
            for (auto v : robj->getParameters())
            {
                tmpObj.addToParameters(v);
            }
            lval.addToParticleIDs(tmpObj);
        }

        // fill the shape parameters
        for (auto v : rval->getShape())
        {
            lval.addToShapeParameters(v);
        }

        // fill the hit contributions
        for (auto v : rval->getHitContributions())
        {
            lval.addToHitContributions(v);
        }

        // fill the Sub-detector energies
        for (auto v : rval->getSubdetectorEnergies())
        {
            lval.addToSubdetectorEnergies(v);
        }
    }

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        // find and set the associated clusters
        for (auto &robj : ((EVENT::Cluster *)src->getElementAt(i))->getClusters())
        {
            int idx = getIndexOf(robj, src);
            auto lobj = idx < 0
                ? getCorresponding<edm4hep::Cluster, edm4hep::ClusterCollection, EVENT::Cluster>("Cluster", robj)
                : dest->at(idx);
            if (lobj.isAvailable())
            {
                dest->at(i).addToClusters(lobj);
            }
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvVertexCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::VertexCollection();

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::Vertex *rval = (EVENT::Vertex *)src->getElementAt(i);
        edm4hep::Vertex lval = dest->create();

        lval.setPrimary(rval->isPrimary() ? 1 : 0);  //1 for primary and 0 for not primary
        lval.setChi2(rval->getChi2());
        lval.setProbability(rval->getProbability());
        lval.setPosition(rval->getPosition());
        auto &m = rval->getCovMatrix();  //6 parameters
        lval.setCovMatrix({m[0], m[1], m[2], m[3], m[4], m[5]});
        //FIXME: the algorithm type in LCIO is a string, but an integer is expected
        //lval.setAlgorithmType(rval->getAlgorithmType());
        //lval.setAssociatedParticle();  //set it when convert ReconstructedParticle

        for (auto v : rval->getParameters())
        {
            lval.addToParameters(v);
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvReconstructedParticleCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::ReconstructedParticleCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
        if (v.second->getTypeName() == "Cluster")
            getCollection(v.first);
    });
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
        if (v.second->getTypeName() == "Track")
            getCollection(v.first);
    });
    // get the ParticleID collection
    if ( m_name2dest.find("ParticleID_EXT") == m_name2dest.end() ) {
        auto tmpCol = new edm4hep::ParticleIDCollection();
        tmpCol->setID(m_table->add("ParticleID_EXT"));
        m_name2dest.insert(std::make_pair("ParticleID_EXT", tmpCol));
    }
    auto particleIdCol = dynamic_cast<edm4hep::ParticleIDCollection*>(m_name2dest["ParticleID_EXT"]);
    // get the Vertex collection
    if ( m_name2dest.find("Vertex_EXT") == m_name2dest.end() ) {
        auto tmpCol = new edm4hep::VertexCollection();
        tmpCol->setID(m_table->add("Vertex_EXT"));
        m_name2dest.insert(std::make_pair("Vertex_EXT", tmpCol));
    }
    auto vertexCol = dynamic_cast<edm4hep::VertexCollection*>(m_name2dest["Vertex_EXT"]);

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::ReconstructedParticle *rval = (EVENT::ReconstructedParticle *)src->getElementAt(i);
        edm4hep::ReconstructedParticle lval = dest->create();

        lval.setCharge(rval->getCharge());
        auto &m = rval->getCovMatrix(); //10 parameters
        lval.setCovMatrix({m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9]});
        lval.setEnergy(rval->getEnergy());
        lval.setGoodnessOfPID(rval->getGoodnessOfPID());
        lval.setMass(rval->getMass());
        lval.setMomentum(Vector3fFrom(rval->getMomentum()));
        lval.setReferencePoint(rval->getReferencePoint());
        lval.setType(rval->getType());

        // generate and set the associated start vertex
        auto rvtx = rval->getStartVertex();
        if ( rvtx ) {
            auto &vm = rval->getCovMatrix(); //6 parameters
            int algType = 0;  //FIXME: in LCIO this is a string
            edm4hep::Vertex startVtx(
                    rvtx->isPrimary() ? 1 : 0, rvtx->getChi2(), rvtx->getProbability(), rvtx->getPosition(),
                    {vm[0], vm[1], vm[2], vm[3], vm[4], vm[5]}, algType);
            startVtx.setAssociatedParticle(lval);
            for (auto v : rvtx->getParameters())
            {
                startVtx.addToParameters(v);
            }
            lval.setStartVertex(startVtx);
            vertexCol->push_back(startVtx);
        }

        // find and set the associated clusters
        auto &rclusters = rval->getClusters();
        for (auto &robj : rclusters)
        {
            auto lobj =
                getCorresponding<edm4hep::Cluster, edm4hep::ClusterCollection,
                                 EVENT::Cluster>("Cluster", robj);
            if (lobj.isAvailable())
            {
                lval.addToClusters(lobj);
            }
        }

        // find and set the associated tracks
        auto &rtrks = rval->getTracks();
        for (auto &robj : rtrks)
        {
            auto lobj =
                getCorresponding<edm4hep::Track, edm4hep::TrackCollection,
                                 EVENT::Track>("Track", robj);
            if (lobj.isAvailable())
            {
                lval.addToTracks(lobj);
            }
        }

        // generate and set the associated ParticleIDs and ParticleIDUsed
        auto rparIDUsed = rval->getParticleIDUsed();
        auto &rparIDs = rval->getParticleIDs();
        for (auto &robj : rparIDs)
        {
            //edm4hep::ParticleID tmpObj(robj->getType(), robj->getPDG(), robj->getAlgorithmType(), robj->getLikelihood());
            edm4hep::ParticleID tmpObj = particleIdCol->create();
            tmpObj.setType( robj->getType() );
            tmpObj.setPDG( robj->getPDG() );
            tmpObj.setAlgorithmType( robj->getAlgorithmType() );
            tmpObj.setLikelihood( robj->getLikelihood() );
            for (auto v : robj->getParameters())
            {
                tmpObj.addToParameters(v);
            }
            lval.addToParticleIDs(tmpObj);
            if (rparIDUsed == robj) {
                lval.setParticleIDUsed(tmpObj);
            }
        }
    }

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        // find and set the associated particles
        for (auto &robj : ((EVENT::ReconstructedParticle *)src->getElementAt(i))->getParticles())
        {
            int idx = getIndexOf(robj, src);
            auto lobj = idx < 0
                ? getCorresponding<edm4hep::ReconstructedParticle, edm4hep::ReconstructedParticleCollection,
                                 EVENT::ReconstructedParticle>("ReconstructedParticle", robj)
                : dest->at(idx);
            if (lobj.isAvailable())
            {
                dest->at(i).addToParticles(lobj);
            }
        }
    }

    return dest;
}

podio::CollectionBase *k4LCIOConverter::cnvAssociationCollection(EVENT::LCCollection *src)
{
    unsigned nTotal = src->getNumberOfElements();
    if (nTotal == 0) {
        return nullptr;
    }

    podio::CollectionBase* result = nullptr;

    auto& para = src->getParameters();
    auto& fromType = para.getStringVal("FromType");
    auto& toType = para.getStringVal("ToType");

    if ( fromType == "ReconstructedParticle" && toType == "MCParticle" ) {
        // get all collections that this collection depends on
        for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
            if (v.second->getTypeName() == "MCParticle")
                getCollection(v.first);
        });
        for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
            if (v.second->getTypeName() == "ReconstructedParticle")
                getCollection(v.first);
        });

        auto dest = new edm4hep::MCRecoParticleAssociationCollection();

        // here is the concrete convertions
        for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
        {
            auto rval = (EVENT::LCRelation *)src->getElementAt(i);
            if((EVENT::ReconstructedParticle *)rval->getFrom()==0 || (EVENT::MCParticle *)rval->getTo()==0) continue;//remove 0
            edm4hep::MCRecoParticleAssociation lval = dest->create();

            // find and set the associated data objects
            auto rFrom = (EVENT::ReconstructedParticle *)rval->getFrom();
            auto lFrom =
                getCorresponding<edm4hep::ReconstructedParticle, edm4hep::ReconstructedParticleCollection,
                                 EVENT::ReconstructedParticle>("ReconstructedParticle", rFrom);
            lval.setRec(lFrom);

            auto rTo = (EVENT::MCParticle *)rval->getTo();
            auto lTo =
                getCorresponding<edm4hep::MCParticle, edm4hep::MCParticleCollection,
                                 EVENT::MCParticle>("MCParticle", rTo);
            lval.setSim(lTo);

            lval.setWeight(rval->getWeight());
        }

        result = dest;
    }
    else if ( fromType == "CalorimeterHit" && toType == "SimCalorimeterHit" ) {
        // get all collections that this collection depends on
        for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
            if (v.second->getTypeName() == "CalorimeterHit")
                getCollection(v.first);
        });
        for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
            if (v.second->getTypeName() == "SimCalorimeterHit")
                getCollection(v.first);
        });

        auto dest = new edm4hep::MCRecoCaloAssociationCollection();

        // here is the concrete convertions
        for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
        {
            auto rval = (EVENT::LCRelation *)src->getElementAt(i);
            if((EVENT::CalorimeterHit *)rval->getFrom()==0 || (EVENT::SimCalorimeterHit *)rval->getTo()==0) continue;//remove 0
            edm4hep::MCRecoCaloAssociation lval = dest->create();

            // find and set the associated data objects
            auto rFrom = (EVENT::CalorimeterHit *)rval->getFrom();
            auto lFrom =
                getCorresponding<edm4hep::CalorimeterHit, edm4hep::CalorimeterHitCollection,
                                 EVENT::CalorimeterHit>("CalorimeterHit", rFrom);
            lval.setRec(lFrom);

            auto rTo = (EVENT::SimCalorimeterHit *)rval->getTo();
            auto lTo =
                getCorresponding<edm4hep::SimCalorimeterHit, edm4hep::SimCalorimeterHitCollection,
                                 EVENT::SimCalorimeterHit>("SimCalorimeterHit", rTo);
            lval.setSim(lTo);

            lval.setWeight(rval->getWeight());
        }

        result = dest;
    }
    else if ( fromType == "TrackerHit" && toType == "SimTrackerHit" ) {
        // get all collections that this collection depends on
        for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
            if (v.second->getTypeName() == "TrackerHit")
                getCollection(v.first);
        });
        for_each(m_name2src.begin(), m_name2src.end(), [this](auto &v) {
            if (v.second->getTypeName() == "SimTrackerHit")
                getCollection(v.first);
        });

        auto dest = new edm4hep::MCRecoTrackerAssociationCollection();

        // here is the concrete convertions
        for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
        {
            auto rval = (EVENT::LCRelation *)src->getElementAt(i);
            if((EVENT::TrackerHit *)rval->getFrom()==0 || (EVENT::SimTrackerHit *)rval->getTo()==0) continue;//remove 0
            edm4hep::MCRecoTrackerAssociation lval = dest->create();

            // find and set the associated data objects
            auto rFrom = (EVENT::TrackerHit *)rval->getFrom();
            auto lFrom =
                getCorresponding<edm4hep::TrackerHit, edm4hep::TrackerHitCollection,
                                 EVENT::TrackerHit>("TrackerHit", rFrom);
            lval.setRec(lFrom);

            auto rTo = (EVENT::SimTrackerHit *)rval->getTo();
            auto lTo =
                getCorresponding<edm4hep::SimTrackerHit, edm4hep::SimTrackerHitCollection,
                                 EVENT::SimTrackerHit>("SimTrackerHit", rTo);
            lval.setSim(lTo);

            lval.setWeight(rval->getWeight());
        }

        result = dest;
    }
    else {
        std::cout<<"Error, Don't find correct fromType="<<fromType<<" or toType="<<toType<<", stop here."<<std::endl; 
        throw;
    }

    return result;
}

int k4LCIOConverter::getIndexOf(EVENT::LCObject *lcObj, EVENT::LCCollection *lcCol)
{
    for (unsigned i = 0, M = lcCol->getNumberOfElements(); i < M; ++i)
    {
        if (lcObj == lcCol->getElementAt(i))
        {
            return i;
        }
    }
    return -1;
}
