#include "VideodrommMappingApp.h"
/*
TODO
- 20161019 warps.xml fix values beyond 1.0
- 20161019 warps freeze
- 20160831 imgui quits on Mac when resize
- 0204 thread for loading image sequence
- 2802 list of shaders show/active on mouseover
- 2802 imgui contextual window depending on mouseover
- 2802 imgui vertical : textures
- warp select mix fbo texture
- flip horiz
- check flip H and V (spout also)
- sort fbo names and indexes (warps only 4 or 5 inputs)
- spout texture 10 create shader 10.glsl(ThemeFromBrazil) iChannel0
- warpwrapper handle texture mode 0 for spout (without fbo)
- put sliderInt instead of popups //warps next
- proper slitscan h and v //wip
- proper rotation

tempo 142
bpm = (60 * fps) / fpb

where bpm = beats per min
fps = frames per second
fpb = frames per beat

fpb = 4, bpm = 142
fps = 142 / 60 * 4 = 9.46
*/
void VideodrommMappingApp::prepare(Settings *settings) {
	settings->setWindowSize(1024, 768);
}
void VideodrommMappingApp::setup() {
	// Settings
	mVDSettings = VDSettings::create();
	// Session
#if (defined( CINDER_MSW )|| defined( CINDER_MAC ))
	mVDSession = VDSession::create(mVDSettings);
#endif
	mVDSettings->mStandalone = true;
	mVDSession->getWindowsResolution();
	setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);

	mVDSettings->iResolution.x = mVDSettings->mRenderWidth;
	mVDSettings->iResolution.y = mVDSettings->mRenderHeight;
	mVDSettings->mRenderPosXY = ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY);
	// UI
	mVDUI = VDUI::create(mVDSettings, mVDSession);
	setFrameRate(mVDSession->getTargetFps());

	mFadeInDelay = true;


	gl::Fbo::Format format;
	//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	// UI fbo
	//mUIFbo = gl::Fbo::create(mVDSettings->mMainWindowWidth, mVDSettings->mMainWindowHeight, format.colorTexture());
	mUIFbo = gl::Fbo::create(1000, 800, format.colorTexture());

	// mouse cursor and ui
	mVDSettings->mCursorVisible = false;
	setUIVisibility(mVDSettings->mCursorVisible);
}

void VideodrommMappingApp::cleanup() {
	CI_LOG_V("shutdown");
	ui::Shutdown();
	mVDSettings->save();
	mVDSession->save();
	quit();
}

void VideodrommMappingApp::resize() {
	mVDUI->resize();
	mVDSession->resize();
}

void VideodrommMappingApp::mouseMove(MouseEvent event)
{
	// pass this event to Mix handler
	if (!mVDSession->handleMouseMove(event)) {
		// let your application perform its mouseMove handling here
	}
}

void VideodrommMappingApp::mouseDown(MouseEvent event)
{
	// pass this event to Mix handler
	if (!mVDSession->handleMouseDown(event)) {
		// let your application perform its mouseDown handling here
	}
}

void VideodrommMappingApp::mouseDrag(MouseEvent event)
{
	// pass this event to Mix handler
	if (!mVDSession->handleMouseDrag(event)) {
		// let your application perform its mouseDrag handling here
	}
}

void VideodrommMappingApp::mouseUp(MouseEvent event)
{
	// pass this event to Mix handler
	if (!mVDSession->handleMouseUp(event)) {
		// let your application perform its mouseUp handling here
	}
}

void VideodrommMappingApp::keyDown(KeyEvent event)
{
	// pass this event to Mix handler
	if (!mVDSession->handleKeyDown(event)) {
		// Animation did not handle the key, so handle it here
		switch (event.getCode()) {
		case KeyEvent::KEY_ESCAPE:
			// quit the application
			quit();
		case KeyEvent::KEY_h:
			// mouse cursor
			mVDSettings->mCursorVisible = !mVDSettings->mCursorVisible;
			setUIVisibility(mVDSettings->mCursorVisible);
			break;
		}
	}
}

void VideodrommMappingApp::keyUp(KeyEvent event)
{
	// pass this event to Mix handler
	if (!mVDSession->handleKeyUp(event)) {
		// let your application perform its keyUp handling here

	}
}

void VideodrommMappingApp::update()
{
	mVDSession->setControlValue(30, getAverageFps());
	mVDSession->update();
}
void VideodrommMappingApp::fileDrop(FileDropEvent event)
{
	mVDSession->fileDrop(event);
}
void VideodrommMappingApp::setUIVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}


// Render the UI into the FBO
void VideodrommMappingApp::renderUIToFbo()
{
	//if (getElapsedFrames() % 300 == 0) {

	if (mVDUI->isReady()) {
		// this will restore the old framebuffer binding when we leave this function
		// on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
		// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
		gl::ScopedFramebuffer fbScp(mUIFbo);
		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0), ivec2(mVDSettings->mFboWidth * mVDSettings->mUIZoom, mVDSettings->mFboHeight * mVDSettings->mUIZoom));
		gl::clear();
		gl::color(Color::white());
	}
	//}
	mVDUI->Run("UI", (int)getAverageFps());

}
void VideodrommMappingApp::draw()
{
	getWindow()->setTitle("(" + mVDSettings->sFps + " fps) " + toString(mVDSettings->iBeat) + " Videodromm");

	gl::clear(Color::black());
	if (mFadeInDelay) {
		mVDSettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mVDSession->getFadeInDelay()) {
			mFadeInDelay = false;
			setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);
			setWindowPos(ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY));
			timeline().apply(&mVDSettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	gl::setMatricesWindow(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight, false);
	gl::draw(mVDSession->getMixTexture(), getWindowBounds());

	//imgui
	if (!mVDSettings->mCursorVisible || Warp::isEditModeEnabled())
	{
		return;
	}
	renderUIToFbo();
	gl::draw(mUIFbo->getColorTexture());

}

// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP(VideodrommMappingApp, RendererGl(RendererGl::AA_NONE))
#else
CINDER_APP(VideodrommMappingApp, RendererGl, &VideodrommMappingApp::prepare)
#endif
