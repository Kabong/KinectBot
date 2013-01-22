#include "ofxONI.h"

ofxONI::ofxONI()
{

}

ofxONI::~ofxONI()
{
	context.Shutdown();
}

void ofxONI::setup()
{
	XnStatus rc = XN_STATUS_OK;
	printf("InitFromXmlFile\n");
	rc = context.InitFromXmlFile(SAMPLE_XML_PATH);
	CHECK_RC(rc, "InitFromXml");

	printf("FindExistingNode\n");
	rc = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depthGenerator);
	CHECK_RC(rc, "Find depth generator");

	printf("FindExistingNode\n");
	rc = context.FindExistingNode(XN_NODE_TYPE_USER, userGenerator);	
	CHECK_RC(rc, "Find user generator");
		
	/*
	if (!userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON) ||
		!userGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
	{
		printf("User generator doesn't support either skeleton or pose detection.\n");
		OF_EXIT_APP(0);
	}
	userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
	*/
	
	printf("StartGeneratingAll\n");
	rc = context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	XnCallbackHandle hUserCBs;
	userGenerator.RegisterUserCallbacks(NewUser, LostUser, NULL, hUserCBs);
	
	/*
	XnCallbackHandle hCalibrationCBs, hPoseCBs;
	userGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(CalibrationStarted, CalibrationEnded, NULL, hCalibrationCBs);
	userGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(PoseDetected, NULL, NULL, hPoseCBs);
	*/

	depthGenerator.GetMetaData(depthMetaData);

	width = depthMetaData.XRes();
	height = depthMetaData.YRes();

	tmpGrayPixels = new unsigned char[width * height];
	tmpColorPixels = new unsigned char[width * height * 3];

	depth.allocate(width, height);
	players.allocate(width, height);
}

void ofxONI::update()
{	
	depthGenerator.GetMetaData(depthMetaData);
	userGenerator.GetUserPixels(0, sceneMetaData);
	
	calculateMaps();
	
	context.WaitAndUpdateAll();
}

void ofxONI::calculateMaps()
{
	// Calculate the accumulative histogram

	unsigned int nValue = 0;
	unsigned int nHistValue = 0;
	unsigned int nIndex = 0;
	unsigned int nX = 0;
	unsigned int nY = 0;
	unsigned int nNumberOfPoints = 0;
	const XnDepthPixel* pDepth = depthMetaData.Data();	

	memset(depthHist, 0, MAX_DEPTH*sizeof(float));
	int n = 0;
	for (nY=0; nY < height; nY++)
	{
		for (nX=0; nX < width; nX++, nIndex++)
		{
			nValue = pDepth[nIndex];

			if (nValue != 0)
			{
				depthHist[nValue]++;
				nNumberOfPoints++;
			}						
		}
	}
	
	for (nIndex=1; nIndex < MAX_DEPTH; nIndex++)
	{
		depthHist[nIndex] += depthHist[nIndex-1];
	}

	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex < MAX_DEPTH; nIndex++)
		{
			depthHist[nIndex] = (unsigned int)(256 * (1.0f - (depthHist[nIndex] / nNumberOfPoints)));
		}
	}
	
	const XnLabel* pLabels = sceneMetaData.Data();
	XnLabel label;

	for (int i = 0; i < width * height; i++)
	{			
		nValue = pDepth[i];
		label = pLabels[i];
		XnUInt32 nColorID = label % nColors;
		if (label == 0)
		{
			nColorID = nColors;
		}

		if (nValue != 0) 
		{
			nHistValue = depthHist[nValue];
			tmpGrayPixels[i] = nHistValue;

			tmpColorPixels[i * 3 + 0] = 255 * oniColors[nColorID][0];
			tmpColorPixels[i * 3 + 1] = 255 * oniColors[nColorID][1];
			tmpColorPixels[i * 3 + 2] = 255 * oniColors[nColorID][2];
		}
		else 
		{
			tmpGrayPixels[i] = 0;

			tmpColorPixels[i * 3 + 0] = 0;
			tmpColorPixels[i * 3 + 1] = 0;
			tmpColorPixels[i * 3 + 2] = 0;
		}
	}
	
	depth.setFromPixels(tmpGrayPixels, width, height);
	players.setFromPixels(tmpColorPixels, width, height);
}

void ofxONI::drawDepth(int x, int y, int w, int h)
{
	depth.draw(x, y, w, h);
}

void ofxONI::drawPlayers(int x, int y, int w, int h)
{
	players.draw(x, y, w, h);

	XnUserID aUsers[15];
	XnUInt16 nUsers;
	userGenerator.GetUsers(aUsers, nUsers);
	for (int i = 0; i < nUsers; ++i)
	{
		XnPoint3D com;
		userGenerator.GetCoM(aUsers[i], com);
		depthGenerator.ConvertRealWorldToProjective(1, &com, &com);
		
		ofSetColor(255, 255, 255);
		ofRect(com.X - 2, com.Y - 10, 10, 12);
		ofSetColor(0, 0, 0);		
		ofDrawBitmapString(ofToString((int)aUsers[i]), com.X, com.Y);
	}
}

