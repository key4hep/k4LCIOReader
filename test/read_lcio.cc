#include "K4LCIOReader/K4LCIOReader.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "podio/EventStore.h"

int main()
{
    auto reader = K4LCIOReader();
    reader.open( "lciodata.slcio" );

    //auto store = podio::EventStore();
    //store.setReader(&reader);

    unsigned nEvt = reader.getEntries();
    std::cout << "total events: " << nEvt << std::endl;

    while ( reader.readNextEvent() )
    {
        auto col1 = reader.getCollection<edm4hep::MCParticleCollection>("MCParticle");
        std::cout << "MCParticleCollection size: " << col1->size() << std::endl;
        //std::cout << *col1 << std::endl;
        auto col2 = reader.getCollection<edm4hep::SimTrackerHitCollection>("TPCCollection");
        std::cout << "COILCollection size: " << col2->size() << std::endl;

        delete col1;
        delete col2;
    }

    reader.close();

    return 0;
}
