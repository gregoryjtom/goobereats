// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.
#include <list>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap<KeyType,ValueType>(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	  // C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    double m_maxLoad;
    int m_size;
    int m_buckets;
    
    //find which bucket to place the key and value in:
    unsigned int findBucket(const KeyType& k) const;
    class Node
    {
    public:
        KeyType m_key;
        ValueType m_value;
    };
    std::list<Node*>* m_map;
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::ExpandableHashMap(double maximumLoadFactor): m_maxLoad(maximumLoadFactor),m_size(0),m_buckets(8)
{
    m_map = new std::list<Node*> [8];
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType,ValueType>::~ExpandableHashMap()
{
    for (int i = 0; i < m_buckets; i++)
    {
        if (!m_map[i].empty())
        {
            for (auto p = m_map[i].begin(); p != m_map[i].end(); p++)
            {
                delete (*p);
            }
        }
    }
    delete [] m_map;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::reset()
{
    for (int i = 0; i < m_buckets; i++)
    {
        if (!m_map[i].empty())
        {
            for (auto p = m_map[i].begin(); p != m_map[i].end(); p++)
            {
                delete (*p);
            }
        }
    }
    delete [] m_map;
    m_map = new std::list<Node*> [8];
    m_size = 0;
    m_buckets = 8;
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType,ValueType>::size() const
{
    return m_size;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType,ValueType>::associate(const KeyType& key, const ValueType& value)
{
    ValueType* valueToChange = find(key);
    if (valueToChange)
    {
        *valueToChange = value;
        return;
    }
    m_size++;
    double size = m_size;
    double buckets = m_buckets;
    double load = size/buckets;
    
    // if adding this item will exceed the max load factor, rehash table
    if (load > m_maxLoad)
    {
        std::list<Node*>* new_map;
        int old_bucket_number = m_buckets;
        m_buckets *= 2;
        new_map = new std::list<Node*>[m_buckets];
        for (int i = 0; i < old_bucket_number; i++)
        {
            if (!m_map[i].empty())
            {
                for (auto p = m_map[i].begin(); p != m_map[i].end(); p++)
                {
                    unsigned int new_bucket = findBucket((*p)->m_key);
                    new_map[new_bucket].push_back(*p);
                }
            }
        }
             
        delete [] m_map;
        m_map = new_map;
    }
    
    unsigned int bucket_num = findBucket(key);

    Node* new_association;
    new_association = new Node;
    new_association->m_key = key;
    new_association->m_value = value;
    m_map[bucket_num].push_back(new_association);
    
}

// If no association exists with the given key, return nullptr; otherwise,
// return a pointer to the value associated with that key. This pointer can be // used to examine that value, and if the hashmap is allowed to be modified, to // modify that value directly within the map (the second overload enables
// this). Using a little C++ magic, we have implemented it in terms of the
// first overload, which you must implement.

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType,ValueType>::find(const KeyType& key) const
{
    unsigned int bucket_num = findBucket(key);
    
    for (auto p = m_map[bucket_num].begin(); p != m_map[bucket_num].end(); p++)
    {
        if ((*p)->m_key == key)
        {
            return &(*p)->m_value;
        }
    }
    
    return nullptr;
}

template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType,ValueType>::findBucket(const KeyType& k) const
{
    unsigned int hasher(const KeyType& k); // prototype
    unsigned int h = hasher(k);
    unsigned int bucket_num = h % m_buckets;
    return bucket_num;
}
