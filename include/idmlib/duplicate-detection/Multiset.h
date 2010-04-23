/**
 * @file Multiset.h
 * @brief The definition and implementation file of the  Multiset class.
 * @author Jinglei
 * @date 2009.11.24
 */

#ifndef Multiset_h
#define Multiset_h 1

#include <UHashTable.h>
#include <PrimeHashTable.h>
#include <wiselib/DArray.h>
#include <fstream>
#include <iostream>

namespace wiselib {

  /**
   * @brief class MultisetEntry contains object and counter which as a unit are put into the
   *  Hashtable.
   *
   * This MultisetEntry objects are data and MultisetEntry.data is key
   *   in Hashtable.
   */
  template <class KeyType, class DataType>
  class MultisetEntry {
  public:
    DataType data;
    int count;
    /**
     * @brief MultisetEntry constructor.
     */
    MultisetEntry() : count(0) {}

    /**
     * @brief MultisetEntry constructor
     *
     * Initialize the object and count.
     */
    MultisetEntry(const DataType& d, int c) {
      data = d;
      count = c;
    }

    /**
     * @brief MultisetEntry constructor
     *
     * Takes only one argument and initialize count to be 1
     */
    MultisetEntry(const DataType& d) {
      data = d;
      count = 1;
    }

    const KeyType& get_key() const { return data.get_key(); }
    int contains_key(const KeyType& k) const { return k == data.get_key(); }

    bool operator==(const MultisetEntry& src) const { return data == src.data; }
    bool operator!=(const MultisetEntry& src) const { return !operator==(src); }

    MultisetEntry& operator=(const MultisetEntry& src) {
      if (this != &src) {
	data = src.data;
	count = src.count;
      }
      return *this;
    }

    friend
    ostream& operator<<(ostream& stream, const MultisetEntry<KeyType, DataType>& me) {
      stream << me.data << "(" << me.count << ")" << endl;
      return stream;
    }
  };

  /**
   * @brief The definition and implementation of the  Multiset class.
   */
  template <class KeyType, class DataType>
  class Multiset {
  private:
    PrimeHashTable<KeyType, MultisetEntry<KeyType, DataType> > hashtable;

    // pix interface.
    PixHT<KeyType, MultisetEntry<KeyType, DataType> > pix;

  public:
    /**
     * @brief set the pix to initial state.
     */
    void set_pix();
    // is pix usable?
    bool is_pix() { return (bool) pix; }
    // next pix
    void next_pix() { pix++; }

    /**
     * @brief MultiSet constructor
     *
     * Creates Hashtable.
     */
    Multiset(int estimatedNum) : hashtable(estimatedNum) { }
    ~Multiset() {} //  this is where the segmentation falut occurrs...
    void release() { hashtable.release(); }

    /**
     * @brief Returns the total number of elements in the multiset.
     *   this count includes duplicates.
     */
    int num_elements() {
      int count = 0;

      PixHT<KeyType, MultisetEntry<KeyType, DataType> > p;
      for (hashtable.set_pix(p); p; p++)
	count += hashtable[p]->count;

      return count;
    }

    /**
     * @brief Returns the number of distinct elements in the multiset.
     *  duplicates are only counted once.
     */
    int num_distinct_elements() const { return hashtable.num_items();}


    /**
     * @brief Returns the count of the number of times that the given
     *  object appears in the multiset. Will be zero if no object is found
     */
    int count_occurrences(const KeyType& key) const {
      const MultisetEntry<KeyType, DataType>* me = hashtable.find(key);
      if (me)
	return me->count;
      else
	return 0;
    }

    /**
     * @brief Returns the data.
     */
    const DataType* get(const KeyType& key) const {
      const MultisetEntry<KeyType, DataType>* me = hashtable.find(key);
      if (me)
	return &me->data;
      else
	return NULL;
    }


    DataType* get(const KeyType& key) {
      MultisetEntry<KeyType, DataType>* me = hashtable.find(key);
      if (me)
	return &me->data;
      else
	return NULL;
    }


    /**
     * @brief add the object to MultiSet.
     *
     *  first create a hte and add it. If found already such an object,
     *  simply increase the count.
     */
    void add(const DataType& toAdd) {
      add(toAdd, 1); // call the two argument version
    }

    /**
     * @brief Takes two areguments. increase object by the numOccurences.
     */
    void add(const DataType& toAdd, int numOccurrences) {
      MultisetEntry<KeyType, DataType>* me = hashtable.find(toAdd.get_key());

      if (me == NULL)
	hashtable.insert(MultisetEntry<KeyType, DataType>(toAdd, numOccurrences));
      else
	me->count += numOccurrences; // simply increase the count
    }

    /* remove
     * ------
     * Removes an element from the multiset, just one occurrence.
     */
    void remove(const KeyType& toRemove) { remove(toRemove, 1); }

    /**
     * @brief Removes an element from the multiset multiple occurrences.
     */
    void remove(const KeyType& toRemove, int numOccurrences) {
      // check for illegal argument
      if (numOccurrences > 0) { // illegal aregument, no operation.
	MultisetEntry<KeyType, DataType>* me = hashtable.find(toRemove);

	if (me != NULL) {
	  me->count -= numOccurrences; // simply decrease the count
	  if (me->count <= 0)  // less than zero, just delete this element
	    hashtable.del(toRemove);
	}
      }
    }

    /**
     * @brief Removes an element and all its occurrences from the multiset.
     *  If no such element is found, no chnages are made
     */
    void remove_all(const KeyType& toRemove) {
      MultisetEntry<KeyType, DataType>* me = hashtable.find(toRemove);

      if (me != NULL)
	hashtable.del(toRemove);
    }

    const DataType* get_pix_data() const { return &hashtable[pix]->data; }
    DataType* get_pix_data() { return &hashtable[pix]->data; }

    const MultisetEntry<KeyType, DataType>* get_pix_me() const {
      return hashtable[pix]; }
    MultisetEntry<KeyType, DataType>* get_pix_me() { return hashtable[pix]; }
  };


  template <class KeyType, class DataType>
  void Multiset<KeyType, DataType>::set_pix() { hashtable.set_pix(pix); }
};
#endif