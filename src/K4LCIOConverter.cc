#include "K4LCIOConverter.h"

//LCIO headers
#include "EVENT/MCParticle.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/TPCHit.h"
#include "EVENT/TrackerHit.h"
#include "EVENT/Track.h"
#include "EVENT/SimCalorimeterHit.h"
#include "EVENT/RawCalorimeterHit.h"
#include "EVENT/CalorimeterHit.h"
#include "EVENT/Cluster.h"
#include "EVENT/ReconstructedParticle.h"
#include "EVENT/LCRelation.h"

//EDM4hep headers
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TPCHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/RawCalorimeterHitCollection.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoParticleAssociationCollection.h"

K4LCIOConverter::K4LCIOConverter(podio::CollectionIDTable *table)
    : m_table(table)
{
    m_cnv["MCParticle"] = &K4LCIOConverter::cnvMCParticleCollection;
    m_cnv["SimTrackerHit"] = &K4LCIOConverter::cnvSimTrackerHitCollection;
    m_cnv["TPCHit"] = &K4LCIOConverter::cnvTPCHitCollection;
    m_cnv["TrackerHit"] = &K4LCIOConverter::cnvTrackerHitCollection;
    m_cnv["Track"] = &K4LCIOConverter::cnvTrackCollection;
    m_cnv["SimCalorimeterHit"] = &K4LCIOConverter::cnvSimCalorimeterHitCollection;
    m_cnv["RawCalorimeterHit"] = &K4LCIOConverter::cnvRawCalorimeterHitCollection;
    m_cnv["CalorimeterHit"] = &K4LCIOConverter::cnvCalorimeterHitCollection;
    m_cnv["Cluster"] = &K4LCIOConverter::cnvClusterCollection;
    m_cnv["ReconstructedParticle"] = &K4LCIOConverter::cnvReconstructedParticleCollection;
    m_cnv["LCRelation"] = &K4LCIOConverter::cnvAssociationCollection;
}

K4LCIOConverter::~K4LCIOConverter()
{
}

void K4LCIOConverter::set(EVENT::LCEvent *evt)
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

podio::CollectionBase *K4LCIOConverter::getCollection(const std::string &name)
{
    // if already exist
    auto idest = m_name2dest.find(name);
    if (idest != m_name2dest.end())
    {
        return idest->second;
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
    m_type2cols[name].push_back(std::make_pair(src, dest));

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

podio::CollectionBase *K4LCIOConverter::cnvMCParticleCollection(EVENT::LCCollection *src)
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

podio::CollectionBase *K4LCIOConverter::cnvSimTrackerHitCollection(EVENT::LCCollection *src)
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

podio::CollectionBase *K4LCIOConverter::cnvTPCHitCollection(EVENT::LCCollection *src)
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

podio::CollectionBase *K4LCIOConverter::cnvTrackerHitCollection(EVENT::LCCollection *src)
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

podio::CollectionBase *K4LCIOConverter::cnvTrackCollection(EVENT::LCCollection *src)
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

    return dest;
}

podio::CollectionBase *K4LCIOConverter::cnvSimCalorimeterHitCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::SimCalorimeterHitCollection();

    // get all collections that this collection depends on
    //for_each(m_name2src.begin(), m_name2src.end(), [this](auto& v) {
    //        if (v.second->getTypeName() == "MCParticle") getCollection(v.first);
    //        });

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::SimCalorimeterHit *rval = (EVENT::SimCalorimeterHit *)src->getElementAt(i);
        edm4hep::SimCalorimeterHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setEnergy(rval->getEnergy());
        lval.setPosition(rval->getPosition());

        // the CaloHitContribution objects are untracked here... Any solution?
        for (auto j = 0, total = rval->getNMCContributions(); j < total; ++j)
        {
            lval.addToContributions(edm4hep::ConstCaloHitContribution(
                rval->getPDGCont(j), rval->getEnergyCont(j), rval->getTimeCont(j), rval->getStepPosition(j)));
        }

        // FIXME: the interface for setting MCParticle is not present in edm4hep now
    }

    return dest;
}

podio::CollectionBase *K4LCIOConverter::cnvRawCalorimeterHitCollection(EVENT::LCCollection *src)
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

podio::CollectionBase *K4LCIOConverter::cnvCalorimeterHitCollection(EVENT::LCCollection *src)
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

podio::CollectionBase *K4LCIOConverter::cnvClusterCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::ClusterCollection();

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

        //TODO
        //lval.addCluster();
        //lval.addHit();
        //lval.addHitContribution();
        //lval.addParticleID();
        //lval.addShap();
        //lval.addSubdetectorEnergie();
        //lval.addWeigh();
    }

    return dest;
}

podio::CollectionBase *K4LCIOConverter::cnvReconstructedParticleCollection(EVENT::LCCollection *src)
{
    auto dest = new edm4hep::ReconstructedParticleCollection();

    for (unsigned i = 0, N = src->getNumberOfElements(); i < N; ++i)
    {
        EVENT::ReconstructedParticle *rval = (EVENT::ReconstructedParticle *)src->getElementAt(i);
        edm4hep::ReconstructedParticle lval = dest->create();

        lval.setCharge(rval->getCharge());
        auto &m = rval->getCovMatrix();
        lval.setCovMatrix({m[0], m[1], m[2], m[3], m[4], m[5]});
        lval.setEnergy(rval->getEnergy());
        lval.setGoodnessOfPID(rval->getGoodnessOfPID());
        lval.setMass(rval->getMass());
        lval.setMomentum(Vector3fFrom(rval->getMomentum()));
        lval.setReferencePoint(rval->getReferencePoint());
        lval.setType(rval->getType());

        //TODO:
        //lval.setStartVertex( rval->getStartVertex() );
        //lval.setParticleIDUsed( rval->getParticleIDUsed() );
        //lval.addTrack();
        //lval.addCluster();
        //lval.addParticle();
        //lval.addParticleID();
    }

    return dest;
}

podio::CollectionBase *K4LCIOConverter::cnvAssociationCollection(EVENT::LCCollection *src)
{
    //TODO
    return nullptr;
}
