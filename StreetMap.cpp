#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <functional>
#include <iostream> // needed for any I/O
#include <fstream>  // needed in addition to <iostream> for file I/O
#include <sstream>  // needed in addition to <iostream> for string stream I/O
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
    
private:
    ExpandableHashMap<GeoCoord,vector<StreetSegment>>* m_hashmap;// add in hash table
    // add in reverse function
};

StreetMapImpl::StreetMapImpl()
{
    m_hashmap = new ExpandableHashMap<GeoCoord,vector<StreetSegment>>;
}

StreetMapImpl::~StreetMapImpl()
{
    delete m_hashmap;
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream inf(mapFile);
    if (!inf)
        return false;
    int i = 0;
    string streetName;
    string line;
    bool justTurnedZero = true;
    bool justReadStreet = false;
    while (getline(inf, line))
    {
        istringstream iss(line);
        string nameOrCoord;
        string startLat,startLong,endLat,endLong;
        string name;
        if (justTurnedZero) // would imply that the line is a street name
        {
            streetName = line;// then change the street name to be this line
            justTurnedZero = false;
            justReadStreet = true;
        }
        
        else if (justReadStreet)// if the line is a number indicating how many street segments
        {
            int amt;
            iss >> amt;
            i += amt;
            justReadStreet = false;
        }
        
        else if (iss >> startLat >> startLong >> endLat >> endLong)
        {
            // coordinates
            GeoCoord start(startLat,startLong);
            GeoCoord end(endLat,endLong);
            StreetSegment forward(start,end,streetName); // forward
            StreetSegment reverse(end,start,streetName); // reverse
            
            vector<StreetSegment>* toChange = m_hashmap->find(start);
            if (toChange)
            {
                toChange->push_back(forward);
            }
            else
            {
                vector<StreetSegment> toAdd;
                toAdd.push_back(forward);
                m_hashmap->associate(start,toAdd);
            }
            
            toChange = m_hashmap->find(end);
            if (toChange)
            {
                toChange->push_back(reverse);
            }
            else
            {
                vector<StreetSegment> toAdd;
                toAdd.push_back(reverse);
                m_hashmap->associate(end,toAdd);
            }
            i--;
            if (i == 0)
                justTurnedZero = true;
            
        }
    
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    segs.clear();
    vector<StreetSegment>* toCopy = m_hashmap->find(gc);
    if (!toCopy)
        return false;
    for (auto p = (*toCopy).begin(); p != (*toCopy).end(); p++)
    {
        StreetSegment segmentToAdd = (*p);
        segs.push_back(segmentToAdd);
    }
    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
