/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#ifndef YARP_DEV_RGBDSENSORWRAPPER_RGBDSENSORWRAPPER_H
#define YARP_DEV_RGBDSENSORWRAPPER_RGBDSENSORWRAPPER_H

#include <vector>
#include <iostream>
#include <string>
#include <sstream>

#include <yarp/os/Port.h>
#include <yarp/os/Time.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/os/BufferedPort.h>


#include <yarp/sig/Vector.h>

#include <yarp/dev/Wrapper.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IRGBDSensor.h>
#include <yarp/dev/IVisualParamsImpl.h>
#include <yarp/dev/FrameGrabberControlImpl.h>

// ROS stuff
#include <yarp/os/Node.h>
#include <yarp/os/Publisher.h>
#include <yarp/os/Subscriber.h>
#include <yarp/rosmsg/TickTime.h>
#include <yarp/rosmsg/sensor_msgs/CameraInfo.h>
#include <yarp/rosmsg/sensor_msgs/Image.h>


namespace yarp{
    namespace dev{
        class RGBDSensorWrapper;
        namespace RGBDImpl
        {
            const std::string frameId_param            = "ROS_frame_Id";
            const std::string nodeName_param           = "ROS_nodeName";
            const std::string colorTopicName_param     = "ROS_colorTopicName";
            const std::string depthTopicName_param     = "ROS_depthTopicName";
            const std::string depthInfoTopicName_param = "ROS_depthInfoTopicName";
            const std::string colorInfoTopicName_param = "ROS_colorInfoTopicName";
            class RGBDSensorParser;
        }
    }
}

#define DEFAULT_THREAD_PERIOD   0.03 // s

// Following three definitions would fit better in a header file
// shared between client and server ... where to place it?
constexpr yarp::conf::vocab32_t VOCAB_PROTOCOL_VERSION = yarp::os::createVocab('p', 'r', 'o', 't');
#define RGBD_WRAPPER_PROTOCOL_VERSION_MAJOR 1
#define RGBD_WRAPPER_PROTOCOL_VERSION_MINOR 0



class yarp::dev::RGBDImpl::RGBDSensorParser:    public DeviceResponder
{
private:
    yarp::dev::IRGBDSensor  *iRGBDSensor;
    yarp::dev::Implement_RgbVisualParams_Parser  rgbParser;
    yarp::dev::Implement_DepthVisualParams_Parser depthParser;
    yarp::dev::FrameGrabberControls_Parser fgCtrlParsers;

public:
    RGBDSensorParser();
    virtual ~RGBDSensorParser() {}
    bool configure(IRGBDSensor *interface);
    bool configure(IRgbVisualParams *rgbInterface, IDepthVisualParams *depthInterface);
    bool configure(IFrameGrabberControls *_fgCtrl);
    virtual bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& response) override;
};


/**
 *  @ingroup dev_impl_wrapper
 *
 * \section RGBDSensorWrapper Description of input parameters
 * A Network grabber for kinect-like devices.
 * This device will produce two streams of data through different ports, one for the color frame and the other one
 * for depth image following Framegrabber and IDepthSensor interfaces specification respectively.
 * See they documentation for more details about each interface.
 *
 * This device is paired with its client called RGBDSensorClient to receive the data streams and perform remote operations.
 *
 *   Parameters required by this device are:
 * | Parameter name | SubParameter            | Type    | Units          | Default Value | Required                       | Description                                                                                         | Notes |
 * |:--------------:|:-----------------------:|:-------:|:--------------:|:-------------:|:-----------------------------: |:---------------------------------------------------------------------------------------------------:|:-----:|
 * | period         |      -                  | int     | ms             |   20          | No                             | refresh period of the broadcasted values in ms                                                      | default 20ms |
 * | name           |      -                  | string  | -              |   -           | Yes, unless useROS='only'      | Prefix name of the ports opened by the RGBD wrapper, e.g. /robotName/RGBD                      | Required suffix like '/rpc' will be added by the device      |
 * | subdevice      |      -                  | string  | -              |   -           | alternative to 'attach' action | name of the subdevice to use as a data source                                                       | when used, parameters for the subdevice must be provided as well |
 * | ROS            |      -                  | group   |  -             |   -           | No                             | Group containing parameter for ROS topic initialization                                             | if missing, it is assumed to not use ROS topics |
 * |   -            |  use_ROS                | string  | true/false/only|   -           |  if ROS group is present       | set 'true' to have both yarp ports and ROS topic, set 'only' to have only ROS topic and no yarp port|  - |
 * |   -            |  forceInfoSync          | string  | bool           |   -           |  no                            | set 'true' to force the timestamp on the camera_info message to match the image one                 |  - |
 * |   -            |  ROS_colorTopicName     | string  |  -             |   -           |  if ROS group is present       | set the name for ROS image topic                                                                    | must start with a leading '/' |
 * |   -            |  ROS_depthTopicName     | string  |  -             |   -           |  if ROS group is present       | set the name for ROS depth topic                                                                    | must start with a leading '/' |
 * |   -            |  ROS_colorInfoTopicName | string  |  -             |   -           |  if ROS group is present       | set the name for ROS imageInfo topic                                                                | must start with a leading '/' |
 * |   -            |  ROS_depthInfoTopicName | string  |  -             |   -           |  if ROS group is present       | set the name for ROS depthInfo topic                                                                | must start with a leading '/' |
 * |   -            |  ROS_frame_Id           | string  |  -             |               |  if ROS group is present       | set the name of the reference frame                                                                 |                               |
 * |   -            |  ROS_nodeName           | string  |  -             |   -           |  if ROS group is present       | set the name for ROS node                                                                           | must start with a leading '/' |
 *
 * ROS message type used is sensor_msgs/Image.msg ( http://docs.ros.org/api/sensor_msgs/html/msg/Image.html)
 * Some example of configuration files:
 *
 * Example of configuration file using .ini format.
 *
 * \code{.unparsed}
 * device RGBDSensorWrapper
 * subdevice <RGBDsensor>
 * period 30
 * name /<robotName>/RGBDSensor
 * \endcode
 */

class yarp::dev::RGBDSensorWrapper: public yarp::dev::DeviceDriver,
                                    public yarp::dev::IWrapper,
                                    public yarp::dev::IMultipleWrapper,
                                    public yarp::os::PeriodicThread
{
private:
#ifndef DOXYGEN_SHOULD_SKIP_THIS

    typedef yarp::sig::ImageOf<yarp::sig::PixelFloat>    DepthImage;
    typedef yarp::os::BufferedPort<DepthImage>           DepthPortType;
    typedef yarp::os::BufferedPort<yarp::sig::FlexImage> ImagePortType;
    typedef yarp::os::Publisher<yarp::rosmsg::sensor_msgs::Image>       ImageTopicType;
    typedef yarp::os::Publisher<yarp::rosmsg::sensor_msgs::CameraInfo>  DepthTopicType;
    typedef unsigned int                                 UInt;

    enum SensorType{COLOR_SENSOR, DEPTH_SENSOR};

    template <class T>
    struct param
    {
        param(T& inVar, std::string inName)
        {
            var          = &inVar;
            parname      = inName;
        }
        T*              var;
        std::string     parname;
    };

    std::string colorFrame_StreamingPort_Name;
    std::string depthFrame_StreamingPort_Name;
    ImagePortType         colorFrame_StreamingPort;
    DepthPortType         depthFrame_StreamingPort;

    // One RPC port should be enough for the wrapper in all cases
    yarp::os::Port        rpcPort;
    std::string rpcPort_Name;
    ImageTopicType        rosPublisherPort_color, rosPublisherPort_depth;
    DepthTopicType        rosPublisherPort_colorCaminfo, rosPublisherPort_depthCaminfo;
    yarp::os::Node*       rosNode;
    std::string           nodeName, depthTopicName, colorTopicName, dInfoTopicName, cInfoTopicName, rosFrameId;
    yarp::sig::FlexImage  colorImage;
    DepthImage            depthImage;
    UInt                  nodeSeq;

    // It should be possible to attach this  guy to more than one port, try to see what
    // will happen when receiving 2 calls at the same time (receive one calls while serving
    // another one, it will result in concurrent thread most probably) and buffering issues.
//     sensor::depth::RGBDSensor_RPCMgsParser  RPC_parser;

    //Helper class for RPCs
    yarp::dev::RGBDImpl::RGBDSensorParser        rgbdParser;

    // Image data specs
    // int hDim, vDim;
    double                         period;
    std::string                    sensorId;
    yarp::dev::IRGBDSensor*        sensor_p;
    yarp::dev::IFrameGrabberControls* fgCtrl;
    IRGBDSensor::RGBDSensor_status sensorStatus;
    int                            verbose;
    bool                           use_YARP;
    bool                           use_ROS;
    bool                           forceInfoSync;
    bool                           initialize_YARP(yarp::os::Searchable &config);
    bool                           initialize_ROS(yarp::os::Searchable &config);
    bool                           read(yarp::os::ConnectionReader& connection);

    // Open the wrapper only, the attach method needs to be called before using it
    // Typical usage: yarprobotinterface
    bool                           openDeferredAttach(yarp::os::Searchable& prop);

    // If a subdevice parameter is given, the wrapper will open it and attach to immediately.
    // Typical usage: simulator or command line
    bool                           isSubdeviceOwned;
    yarp::dev::PolyDriver*         subDeviceOwned;
    bool                           openAndAttachSubDevice(yarp::os::Searchable& prop);

    // Synch
    yarp::os::Stamp                colorStamp;
    yarp::os::Stamp                depthStamp;
    yarp::os::Property             m_conf;

    void shallowCopyImages(const yarp::sig::FlexImage& src, yarp::sig::FlexImage& dest);
    void shallowCopyImages(const DepthImage& src, DepthImage& dest);
    bool writeData();
    void deepCopyImages(const yarp::sig::FlexImage&       src,
                        yarp::rosmsg::sensor_msgs::Image& dest,
                        const std::string&                frame_id,
                        const yarp::rosmsg::TickTime&     timeStamp,
                        const UInt&                       seq);

    void deepCopyImages(const DepthImage&                 src,
                        yarp::rosmsg::sensor_msgs::Image& dest,
                        const std::string&                frame_id,
                        const yarp::rosmsg::TickTime&     timeStamp,
                        const UInt&                       seq);

    bool setCamInfo(yarp::rosmsg::sensor_msgs::CameraInfo& cameraInfo,
                    const std::string&                     frame_id,
                    const UInt&                            seq,
                    const SensorType&                      sensorType);

    static std::string yarp2RosPixelCode(int code);



#endif //DOXYGEN_SHOULD_SKIP_THIS

public:
    RGBDSensorWrapper();
    ~RGBDSensorWrapper();

    bool        open(yarp::os::Searchable &params) override;
    bool        fromConfig(yarp::os::Searchable &params);
    bool        close() override;

    void        setId(const std::string &id);
    std::string getId();

    /**
      * Specify which sensor this thread has to read from.
      */
    bool        attachAll(const PolyDriverList &p) override;
    bool        detachAll() override;

    bool        attach(PolyDriver *poly) override;
    bool        attach(yarp::dev::IRGBDSensor *s);
    bool        detach() override;

    bool        threadInit() override;
    void        threadRelease() override;
    void        run() override;
};

#endif   // YARP_DEV_RGBDSENSORWRAPPER_RGBDSENSORWRAPPER_H
