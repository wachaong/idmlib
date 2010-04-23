#include <idmlib/duplicate-detection/RandomProjection.h>
using namespace std;
namespace sf1v5{

/********************************************************************************
  Description: A convenience method for doing element-wise additions.
  Comments   :
********************************************************************************/
void RandomProjection::operator+=(const RandomProjection& rp)
{
    for (int i = 0; i < rp.num_dimensions(); i++) {
        projection[i] += rp[i];
    }
}


/********************************************************************************
  Description:
  Comments   :
 ********************************************************************************/
void RandomProjection::display(ostream& stream) const
{
    cout << "[" << ub8key << "] [" << projection << "]";
}


/********************************************************************************
  Description: Generates a CBitArray object from a random projection.
  Comments   :
 ********************************************************************************/
void RandomProjection::generate_bitarray(const wiselib::DynamicArray<float>& projection,
                                         wiselib::CBitArray& bitArray)
{
    bitArray.ResetAll();
    for (int i = 0; i < projection.size(); i++) {
        if (projection[i] >= 0) {
            bitArray.SetAt(i);
        }
    }
}

}