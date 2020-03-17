#include "K4LCIOConverter.h"

//LCIO headers
#include "EVENT/MCParticle.h"
#include "EVENT/SimTrackerHit.h"
#include "EVENT/TPCHit.h"
#include "EVENT/TrackerHit.h"
#include "EVENT/SimCalorimeterHit.h"
#include "EVENT/LCRelation.h"

//EDM4hep headers
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TPCHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"

K4LCIOConverter::K4LCIOConverter(podio::CollectionIDTable* table)
    : m_table(table)
{
    m_cnv["MCParticle"] = &K4LCIOConverter::cnvMCParticleCollection;
    m_cnv["SimTrackerHit"] = &K4LCIOConverter::cnvSimTrackerHitCollection;
    m_cnv["TPCHit"] = &K4LCIOConverter::cnvTPCHitCollection;
    m_cnv["TrackerHit"] = &K4LCIOConverter::cnvTrackerHitCollection;
    //m_cnv["SimCalorimeterHit"] = &K4LCIOConverter::cnvSimCalorimeterHitCollection;
    m_cnv["LCRelation"] = &K4LCIOConverter::cnvAssociationCollection;
}

K4LCIOConverter::~K4LCIOConverter()
{
}

void K4LCIOConverter::set(EVENT::LCEvent* evt)
{
    m_name2src.clear();
    m_name2dest.clear();
    m_type2cols.clear();

    m_evt = evt;
    for ( const auto& colname : *(evt->getCollectionNames()) ) {
        auto pcol = evt->getCollection(colname);
        m_name2src[colname] = pcol;
    }
}

podio::CollectionBase* K4LCIOConverter::getCollection(const std::string& name)
{
    // if already exist
    auto idest = m_name2dest.find(name);
    if ( idest != m_name2dest.end() ) {
        return idest->second;
    }

    // if not a valid collection
    auto isrc = m_name2src.find(name);
    if ( isrc == m_name2src.end() ) {
        m_name2dest[name] = nullptr;
        return nullptr;
    }

    // if not a known type
    EVENT::LCCollection* src = isrc->second;
    auto it = m_cnv.find(src->getTypeName());
    if ( it == m_cnv.end() ) {
        std::cout << "Error: do not support convertion of " << name
            << " with type " << src->getTypeName()
            << std::endl;
        return nullptr;
    }

    // convert
    podio::CollectionBase* dest = (this->*(it->second))(src);
    dest->setID( m_table->add(name) );

    // put result in data holders
    m_name2dest[name] = dest;
    m_type2cols[name].push_back(std::make_pair(src, dest));

    return dest;
}

static edm4hep::Vector3f Vector3fFrom(const double* v)
{
    return edm4hep::Vector3f(v[0], v[1], v[2]);
}

podio::CollectionBase* K4LCIOConverter::cnvMCParticleCollection(EVENT::LCCollection* src)
{
    auto dest = new edm4hep::MCParticleCollection();

    for( unsigned i = 0, N = src->getNumberOfElements(); i< N; ++i ) {
        EVENT::MCParticle* rval = (EVENT::MCParticle*) src->getElementAt(i) ;
        edm4hep::MCParticle  lval = dest->create();

        lval.setPDG( rval->getPDG() );
        lval.setGeneratorStatus( rval->getGeneratorStatus() );
        lval.setSimulatorStatus( rval->getSimulatorStatus() );
        lval.setCharge( rval->getCharge() );
        lval.setTime( rval->getTime() );
        lval.setMass( rval->getMass() );
        lval.setSpin( edm4hep::Vector3f( rval->getSpin() ) );
        lval.setColorFlow( edm4hep::Vector2i( rval->getColorFlow() ) );
        lval.setVertex( edm4hep::Vector3d( rval->getVertex() ) );
        lval.setEndpoint( edm4hep::Vector3d( rval->getEndpoint() ) );
        lval.setMomentum( Vector3fFrom( rval->getMomentum() ) );
        lval.setMomentumAtEndpoint( Vector3fFrom( rval->getMomentumAtEndpoint() ) );

        // ? suppose there is only one MCParticleCollection in each event
        for ( auto rparent : rval->getParents() ) {
            for ( unsigned k = 0; k < i; ++k ) {
                if( src->getElementAt(k) == rparent ) {
                    // A loop for edm4hep's MCParticleCollection to recover the relationship
                    edm4hep::MCParticle lparent = dest->at(k);
                    lval.addParent(lparent);
                    lparent.addDaughter(lval);        
                    break;
                }
            }
        }
    }

    return dest;
}

podio::CollectionBase* K4LCIOConverter::cnvSimTrackerHitCollection(EVENT::LCCollection* src)
{
    auto dest = new edm4hep::SimTrackerHitCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto& v) {
            if (v.second->getTypeName() == "MCParticle") getCollection(v.first);
            });

    // fill the collection
    for( unsigned i = 0, N = src->getNumberOfElements(); i< N; ++i ) {
        EVENT::SimTrackerHit* rval = (EVENT::SimTrackerHit*) src->getElementAt(i) ;
        edm4hep::SimTrackerHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID( cellID );
        lval.setEDep( rval->getEDep() );
        lval.setTime( rval->getTime() );
        lval.setPathLength( rval->getPathLength() );
        lval.setQuality( rval->getQuality() );
        lval.setPosition( rval->getPosition() );
        lval.setMomentum( rval->getMomentum() );

        // find and set the associated MCParticle
        const EVENT::MCParticle* robj = rval->getMCParticle();
        if ( ! robj ) continue;
        bool found = false;
        for ( auto& colpair : m_type2cols["MCParticle"] ) {
            auto rcol = colpair.first;
            for( unsigned j = 0, M = rcol->getNumberOfElements(); j < M; ++j ) {
                if ( robj == rcol->getElementAt(j) ) {
                    found = true;
                    auto lcol = dynamic_cast<edm4hep::MCParticleCollection*>(colpair.second);
                    lval.setMCParticle( lcol->at(j) );
                    break;
                }
            }
            if ( found ) break;
        }
    }

    return dest;
}

podio::CollectionBase* K4LCIOConverter::cnvTPCHitCollection(EVENT::LCCollection* src)
{
    auto dest = new edm4hep::TPCHitCollection();

    for( unsigned i = 0, N = src->getNumberOfElements(); i< N; ++i ) {
        EVENT::TPCHit* rval = (EVENT::TPCHit*) src->getElementAt(i);
        edm4hep::TPCHit lval = dest->create();

        lval.setCellID( rval->getCellID() );
        lval.setTime( rval->getTime() );
        lval.setCharge( rval->getCharge() );
        lval.setQuality( rval->getQuality() );
        for( unsigned j = 0, M = rval->getNRawDataWords(); j < M; j++ ) {
            lval.addRawDataWord( rval->getRawDataWord(j) );
        }
    }

    return dest;
}

podio::CollectionBase* K4LCIOConverter::cnvTrackerHitCollection(EVENT::LCCollection* src)
{
    auto dest = new edm4hep::TrackerHitCollection();

    // get all collections that this collection depends on
    for_each(m_name2src.begin(), m_name2src.end(), [this](auto& v) {
            if (v.second->getTypeName() == "TPCHit") getCollection(v.first);
            });

    // fill the collection
    for( unsigned i = 0, N = src->getNumberOfElements(); i< N; ++i ) {
        EVENT::TrackerHit* rval = (EVENT::TrackerHit*) src->getElementAt(i);
        edm4hep::TrackerHit lval = dest->create();

        unsigned long long cellID = rval->getCellID1();
        cellID = (cellID << 32) | rval->getCellID0();
        lval.setCellID(cellID);
        lval.setType( rval->getType() );
        lval.setQuality( rval->getQuality() );
        lval.setTime( rval->getTime() );
        lval.setEDep( rval->getEDep() );
        lval.setEDepError( rval->getEDepError() );
        lval.setEdx( rval->getdEdx() );
        lval.setPosition( rval->getPosition() );
        auto& m = rval->getCovMatrix();
        lval.setCovMatrix( { m[0], m[1], m[2], m[3], m[4], m[5] } );

        // find and set the associated RawHit ( only TPCHit is considered now )
        auto& robjvec = rval->getRawHits();
        for ( auto& robj : robjvec ) {
            bool found = false;
            for ( auto& colpair : m_type2cols["TPCHit"] ) {
                auto rcol = colpair.first;
                for( unsigned j = 0, M = rcol->getNumberOfElements(); j < M; ++j ) {
                    if ( robj == rcol->getElementAt(j) ) {
                        found = true;
                        auto lcol = dynamic_cast<edm4hep::TPCHitCollection*>(colpair.second);
                        lval.addRawHit( lcol->at(j).getObjectID() );
                        break;
                    }
                }
                if ( found ) break;
            }
        }
    }

    return dest;
}

//podio::CollectionBase* K4LCIOConverter::cnvSimCalorimeterHitCollection(EVENT::LCCollection* src)
//{
//    auto dest = new edm4hep::SimCalorimeterHitCollection();
//
//    for( unsigned i = 0, N = src->getNumberOfElements(); i< N; ++i ) {
//        EVENT::SimCalorimeterHit* rval = (EVENT::SimCalorimeterHit*) src->getElementAt(i);
//        edm4hep::SimCalorimeterHit lval = dest->create();
//    }
//
//    return dest;
//}

podio::CollectionBase* K4LCIOConverter::cnvAssociationCollection(EVENT::LCCollection* src)
{
    return nullptr;
}
