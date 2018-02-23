#ifndef YARP_SIG_POINTCLOUD_NETWORKHEADER_H
#define YARP_SIG_POINTCLOUD_NETWORKHEADER_H

#include <yarp/conf/system.h>

#include <yarp/os/NetInt32.h>
#include <yarp/os/Portable.h>
#include <yarp/os/Bottle.h>
#include <yarp/sig/Vector.h>
#include <map>
#include <yarp/sig/PointCloudTypes.hpp>

namespace yarp {
    namespace sig {
        class PointCloud_NetworkHeader;
    }
}

// Plain xy point is not supported for now ... does it make sense to have it?
// How to let the user to add his own type?

// Define a bit for each piece of information we want to carry
// Is enum better?? Define some helper to get a string from number
#define   PC_XY_DATA                        (1 << 0)
#define   PC_XYZ_DATA                       (1 << 1)
#define   PC_RGBA_DATA                      (1 << 2)
#define   PC_INTENSITY_DATA                 (1 << 3)
#define   PC_INTEREST_DATA                  (1 << 4)   // in PCL this field is also called strenght
#define   PC_NORMAL_DATA                    (1 << 5)
#define   PC_CURVATURE_DATA                 (1 << 6)
#define   PC_RANGE_DATA                     (1 << 7)
#define   PC_VIEWPOINT_DATA                 (1 << 8)
#define   PC_MOMENT_INV_DATA                (1 << 9)
#define   PC_RADII_RSD_DATA                 (1 << 10)
#define   PC_BOUNDARY_DATA                  (1 << 11)
#define   PC_PRINCIPAL_CURVATURE_DATA       (1 << 12)
#define   PC_PFH_SIGNAT_125_DATA            (1 << 13)
#define   PC_FPFH_SIGNAT_33_DATA            (1 << 14)
#define   PC_VFH_SIGNAT_308_DATA            (1 << 15)
#define   PC_NARF_36_DATA                   (1 << 16)
#define   PC_BORDER_DATA                    (1 << 17)
#define   PC_INTENSITY_GRAD_DATA            (1 << 18)
#define   PC_HISTOGRAM_DATA                 (1 << 19)
#define   PC_SCALE_DATA                     (1 << 20)
#define   PC_CONFIDENCE_DATA                (1 << 21)
#define   PC_RADIUS_DATA                    (1 << 22)
#define   PC_USER_DEFINED                   (1 << 23)
#define   PC_PADDING2                       (1 << 24)
#define   PC_PADDING3                       (1 << 25)

// Shortcuts names for matching PCL predefined types
#define   PCL_POINT2D_XY                (PC_XY_DATA)
#define   PCL_POINT_XYZ                 (PC_XYZ_DATA)

#define   PCL_POINT_XYZ_RGBA            (PC_XYZ_DATA | PC_RGBA_DATA | PC_PADDING3)
#define   PCL_POINT_XYZ_I               (PC_XYZ_DATA | PC_INTENSITY_DATA)

#define   PCL_INTEREST_POINT_XYZ        (PC_XYZ_DATA | PC_INTEREST_DATA)
#define   PCL_NORMAL                    (PC_NORMAL_DATA | PC_CURVATURE_DATA | PC_PADDING3)
#define   PCL_POINT_XYZ_NORMAL          (PC_XYZ_DATA | PC_NORMAL_DATA | PC_CURVATURE_DATA | PC_PADDING3)
#define   PCL_POINT_XYZ_NORMAL_RGBA     (PC_XYZ_DATA | PC_RGBA_DATA | PC_NORMAL_DATA | PC_CURVATURE_DATA | PC_PADDING2)   // Actually PCL has PointXYZRGBNormal, not RGBA, but downgrade from rgba to rgb can be done while casting
#define   PCL_POINT_XYZ_I_NORMAL        (PC_XYZ_DATA | PC_INTENSITY_DATA | PC_NORMAL_DATA | PC_CURVATURE_DATA)
#define   PCL_POINT_XYZ_RANGE           (PC_XYZ_DATA | PC_RANGE_DATA)
#define   PCL_POINT_XYZ_VIEWPOINT       (PC_XYZ_DATA | PC_VIEWPOINT_DATA)
#define   PCL_MOMENT_INVARIANTS         (PC_MOMENT_INV_DATA)
#define   PCL_PRINCIPAL_RADII_RSD       (PC_RADII_RSD_DATA)
#define   PCL_BOUNDARY                  (PC_BOUNDARY_DATA)
#define   PCL_PRINCIPAL_CURVATURES      (PC_PRINCIPAL_CURVATURE_DATA)
#define   PCL_PFH_SIGNAT_125            (PC_PFH_SIGNAT_125_DATA)
#define   PCL_FPFH_SIGNAT_33            (PC_FPFH_SIGNAT_33_DATA)
#define   PCL_VFH_SIGNAT_308            (PC_VFH_SIGNAT_308_DATA)
#define   PCL_NARF_36                   (PC_NARF_36_DATA)
#define   PCL_POINT2D_BORDER            (PC_XY_DATA | PC_BORDER_DATA)
#define   PCL_INTENSITY_GRADIENT        (PC_INTENSITY_GRAD_DATA)
#define   PCL_PC_HISTOGRAM_N            (PC_HISTOGRAM_DATA)
#define   PCL_POINT_XYZ_SCALE           (PC_XYZ_DATA | PC_SCALE_DATA)
#define   PCL_POINT_XYZ_SURFEL          (PC_XYZ_DATA | PC_RGBA_DATA | PC_NORMAL_DATA | PC_RADIUS_DATA | PC_CONFIDENCE_DATA)


std::map<std::pair<int, int>, int> offsetMap = {
    // PCL_NORMAL
    {std::make_pair(PCL_NORMAL, PC_CURVATURE_DATA) , sizeof(NORMAL_NO_CURV)},

    // PCL_POINT_XYZ_RGBA
    {std::make_pair(PCL_POINT_XYZ_RGBA, PC_RGBA_DATA) , sizeof(XYZ_DATA)},

    // PCL_POINT_XYZ_I
    {std::make_pair(PCL_POINT_XYZ_I, PC_INTENSITY_DATA) , sizeof(XYZ_DATA)},

    // PCL_INTEREST_POINT_XYZ
    {std::make_pair(PCL_INTEREST_POINT_XYZ, PC_INTEREST_DATA) , sizeof(XYZ_DATA)},

    // PCL_POINT_XYZ_NORMAL
    {std::make_pair(PCL_POINT_XYZ_NORMAL, PC_NORMAL_DATA) , sizeof(XYZ_DATA)},
    {std::make_pair(PCL_POINT_XYZ_NORMAL, PC_CURVATURE_DATA) , sizeof(XYZ_DATA) + sizeof(NORMAL_NO_CURV)},

//    // PCL_XYZ_NORMAL_RGBA TBD
    {std::make_pair(PCL_POINT_XYZ_NORMAL_RGBA, PC_NORMAL_DATA) , sizeof(XYZ_DATA)},
    {std::make_pair(PCL_POINT_XYZ_NORMAL_RGBA, PC_RGBA_DATA) , sizeof(XYZ_DATA) + sizeof(NORMAL_NO_CURV)},
    {std::make_pair(PCL_POINT_XYZ_NORMAL_RGBA, PC_CURVATURE_DATA) , sizeof(XYZ_DATA) + sizeof(NORMAL_NO_CURV) + sizeof(RGBA_DATA)},

//    // PCL_XYZ_I_NORMAL TBD
                                 };


std::map<int, std::vector<int> > compositionMap = {
    // recipe for basic data
    {PC_XY_DATA,        std::vector<int> {PC_XY_DATA}},
    {PC_XYZ_DATA,       std::vector<int> {PC_XYZ_DATA}},
    {PC_RGBA_DATA,      std::vector<int> {PC_RGBA_DATA}},
    {PC_INTENSITY_DATA, std::vector<int> {PC_INTENSITY_DATA}},
    {PC_INTEREST_DATA,  std::vector<int> {PC_INTEREST_DATA}},
    {PCL_NORMAL,        std::vector<int> {PC_NORMAL_DATA, PC_CURVATURE_DATA, PC_PADDING3}},
    {PC_NORMAL_DATA,    std::vector<int> {PC_NORMAL_DATA}},
    {PC_CURVATURE_DATA, std::vector<int> {PC_CURVATURE_DATA}},
    {PC_RANGE_DATA,     std::vector<int> {PC_RANGE_DATA}},
    {PC_VIEWPOINT_DATA, std::vector<int> {PC_VIEWPOINT_DATA}},
    // PCL_POINT_XYZ_RGBA
    {PCL_POINT_XYZ_RGBA, std::vector<int> {PC_XYZ_DATA, PC_RGBA_DATA, PC_PADDING3}},
    // PCL_POINT_XYZ_I
    {PCL_POINT_XYZ_I, std::vector<int> {PC_XYZ_DATA, PC_INTENSITY_DATA}},
    // PCL_INTEREST_POINT_XYZ
    {PCL_INTEREST_POINT_XYZ, std::vector<int> {PC_XYZ_DATA, PC_INTEREST_DATA}},
    // PCL_POINT_XYZ_NORMAL
    {PCL_POINT_XYZ_NORMAL, std::vector<int> {PC_XYZ_DATA, PC_NORMAL_DATA, PC_CURVATURE_DATA, PC_PADDING3}},
    // PCL_POINT_XYZ_NORMAL_RGBA
    {PCL_POINT_XYZ_NORMAL_RGBA, std::vector<int> {PC_XYZ_DATA, PC_NORMAL_DATA, PC_RGBA_DATA, PC_CURVATURE_DATA, PC_PADDING2}}

};

// TODO unify the info if possible

std::map<int, size_t> sizeMap = {
    {PC_PADDING3, 3*sizeof(float)},
    {PC_PADDING2, 2*sizeof(float)},
    {PC_XY_DATA, sizeof(XY_DATA)},
    {PC_XYZ_DATA, sizeof(XYZ_DATA)},
    {PC_RGBA_DATA, sizeof(RGBA_DATA)},
    {PC_INTENSITY_DATA, sizeof(intensity)},
    {PC_INTEREST_DATA, sizeof(strength)},
    {PC_NORMAL_DATA, sizeof(NORMAL_NO_CURV)},
    {PCL_NORMAL, sizeof(NORMAL_DATA)},
    {PC_CURVATURE_DATA, sizeof(CURVATURE_DATA)}, //TODO it has different sizes....
    {PC_RANGE_DATA, sizeof(range)},
    {PC_VIEWPOINT_DATA, sizeof(VIEWPOINT_DATA)},
    {PCL_POINT_XYZ_RGBA, sizeof(XYZ_RGBA_DATA)},
    {PCL_POINT_XYZ_I, sizeof(XYZ_I_DATA)},
    {PCL_INTEREST_POINT_XYZ, sizeof(INTEREST_POINT_XYZ_DATA)},
    {PCL_POINT_XYZ_NORMAL, sizeof(XYZ_NORMAL_DATA)}
};
// Defined as in PCL pointTypes.h file for better compatibility
enum BorderTrait
{
    BORDER_TRAIT__OBSTACLE_BORDER,
    BORDER_TRAIT__SHADOW_BORDER,
    BORDER_TRAIT__VEIL_POINT,
    BORDER_TRAIT__SHADOW_BORDER_TOP,
    BORDER_TRAIT__SHADOW_BORDER_RIGHT,
    BORDER_TRAIT__SHADOW_BORDER_BOTTOM,
    BORDER_TRAIT__SHADOW_BORDER_LEFT,
    BORDER_TRAIT__OBSTACLE_BORDER_TOP,
    BORDER_TRAIT__OBSTACLE_BORDER_RIGHT,
    BORDER_TRAIT__OBSTACLE_BORDER_BOTTOM,
    BORDER_TRAIT__OBSTACLE_BORDER_LEFT,
    BORDER_TRAIT__VEIL_POINT_TOP,
    BORDER_TRAIT__VEIL_POINT_RIGHT,
    BORDER_TRAIT__VEIL_POINT_BOTTOM,
    BORDER_TRAIT__VEIL_POINT_LEFT,
};

/**
 *
 * Byte order in Point Cloud header for network transmission.
 *
 */
// YARP_BEGIN_PACK
class yarp::sig::PointCloud_NetworkHeader
{
public:
    PointCloud_NetworkHeader() :    width(10),
                                    height(1),
                                    pointType(0),
                                    isDense(false),
//                                     pose(4, 0.0f),
//                                     orientation(4, 0.0f),
                                    hasCustomData(false),
                                    pad{0,0}
                                    { YARP_UNUSED(pad); }
    // PCL like fields
    yarp::os::NetInt32  width;
    yarp::os::NetInt32  height;
    yarp::os::NetInt32  pointType;       // bitwise of all possible informations -> could also be int64 or just an enum, but I thin bitwise gives more freedom about all possible combinations
    bool                isDense;         // this guy is mis-aligned // convert to (char?) when serialized
//     yarp::sig::Vector   pose;            // translation from origin -- could be an Eigen::Vector4f for better PCL compatibility if yarp can afford to depend from it
//     yarp::sig::Vector   orientation;     // orientation wrt origin  -- could be an Eigen::Quaternion for better PCL compatibility if yarp can afford to depend from it

    // YARPish fileds
//     char                *data;           // actual pointCloud data.

    // Other
    bool                hasCustomData;   // this guy is mis-aligned --> for future use, to have addictional data not initially forseen or fields from ROS which does not fit into yarp pointTypes
//     yarp::os::Bottle    *customData;     // used in case of custom data, to check if a different type can be better optimized for transmission?
private:
    char pad[2]; // char vector to align the bytes of the headers
};
// YARP_END_PACK



#endif // YARP_SIG_POINTCLOUD_NETWORKHEADER_H