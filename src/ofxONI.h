#ifndef __ofxONI_h__
#define __ofxONI_h__

#include "ofMain.h"
#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include "ofxOpenCv.h"


static void XN_CALLBACK_TYPE NewUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("New User id: %i\n", (unsigned int)user);
};

static void XN_CALLBACK_TYPE LostUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("Lost User id: %i\n", (unsigned int)user);
};


void XN_CALLBACK_TYPE PoseDetected(xn::PoseDetectionCapability& pose, const XnChar* strPose, XnUserID user, void* cxt)
{
	printf("Found pose \"%s\" for user %d\n", strPose, user);
	g_UserGenerator.GetSkeletonCap().RequestCalibration(user, TRUE);
	g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(user);
}

void XN_CALLBACK_TYPE CalibrationStarted(xn::SkeletonCapability& skeleton, XnUserID user, void* cxt)
{
	printf("Calibration started\n");
}

void XN_CALLBACK_TYPE CalibrationEnded(xn::SkeletonCapability& skeleton, XnUserID user, XnBool bSuccess, void* cxt)
{
	printf("Calibration done [%d] %ssuccessfully\n", user, bSuccess?"":"un");
	if (bSuccess)
	{
		if (!g_bCalibrated)
		{
			g_UserGenerator.GetSkeletonCap().SaveCalibrationData(user, 0);
			g_nPlayer = user;
			g_UserGenerator.GetSkeletonCap().StartTracking(user);
			g_bCalibrated = TRUE;
		}

		XnUserID aUsers[10];
		XnUInt16 nUsers = 10;
		g_UserGenerator.GetUsers(aUsers, nUsers);
		for (int i = 0; i < nUsers; ++i)
			g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(aUsers[i]);
	}
}


#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		OF_EXIT_APP(0);													\
	}

#define SAMPLE_XML_PATH "data/Sample-User.xml"

#define MAX_DEPTH 10000

static XnFloat oniColors[][3] =
{
	{0,1,1},
	{0,0,1},
	{0,1,0},
	{1,1,0},
	{1,0,0},
	{1,.5,0},
	{.5,1,0},
	{0,.5,1},
	{.5,0,1},
	{1,1,.5},
	{0,0,0}
};
static XnUInt32 nColors = 10;

class ofxONI
{
    public:
        ofxONI();
        ~ofxONI();

		void setup();
		void update();
		
		void drawDepth(int x, int y) {drawDepth(x, y, width, height);};
		void drawDepth(int x, int y, int w, int h);
		void drawPlayers(int x, int y) {drawPlayers(x, y, width, height);};
		void drawPlayers(int x, int y, int w, int h);

		void calculateMaps();

		xn::Context context;
		xn::DepthGenerator depthGenerator;
		xn::UserGenerator userGenerator;

		xn::SceneMetaData sceneMetaData;
		xn::DepthMetaData depthMetaData;

		ofxCvGrayscaleImage depth;
		ofxCvColorImage players;
		
		float depthHist[MAX_DEPTH];

		unsigned char * tmpGrayPixels;
		unsigned char * tmpColorPixels;

		int width, height;
};

#endif

