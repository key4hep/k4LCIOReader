#include "K4LCIOReader/K4LCIOReader.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "podio/EventStore.h"

int main()
{
    auto reader = K4LCIOReader();
    reader.open( "lciodata.slcio" );

    auto store = podio::EventStore();
    store.setReader(&reader);

    unsigned nEvt = reader.getEntries();
    std::cout << "total events: " << nEvt << std::endl;

    while ( reader.readNextEvent() )
    {
        //auto &col1 = store.get<edm4hep::MCParticleCollection>("MCParticle");
        //std::cout << "MCParticleCollection size: " << col1.size() << std::endl;
        //std::cout << col1 << std::endl;

        auto col1 = reader.getCollection<edm4hep::MCParticleCollection>("MCParticle");
        std::cout << "MCParticleCollection size: " << col1->size() << std::endl;
        std::cout << *col1 << std::endl;

        auto col2 = reader.getCollection<edm4hep::SimTrackerHitCollection>("TPCCollection");
        std::cout << "TPCCollection size: " << col2->size() << std::endl;
        std::cout << *col2 << std::endl;

        auto col3 = reader.getCollection<edm4hep::TrackerHitCollection>("SETSpacePoints");
        std::cout << "SETSpacePoints size: " << col3->size() << std::endl;
        std::cout << *col3 << std::endl;

        auto col4 = reader.getCollection<edm4hep::TrackCollection>("MarlinTrkTracks");
        std::cout << "MarlinTrkTracks size: " << col4->size() << std::endl;
        std::cout << *col4 << std::endl;

        delete col1;
        delete col2;
        delete col3;
        delete col4;

        //break;
    }

    reader.close();

    return 0;
}
