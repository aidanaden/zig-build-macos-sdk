/* CoreGraphics - CGDisplayStream.h
   Copyright (c) 2011-2013 Apple Inc.
   All rights reserved. */

#ifndef CGDISPLAYSTREAM_H_
#define CGDISPLAYSTREAM_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFAvailability.h>
#include <stdint.h>
#include <dispatch/dispatch.h>

#include <CoreGraphics/CGDirectDisplay.h>

CF_IMPLICIT_BRIDGING_ENABLED

CF_ASSUME_NONNULL_BEGIN

#ifdef __BLOCKS__

/*!
 @typedef CGDisplayStreamRef
 @abstract An opaque reference to a CGDisplayStream object
 @discussion A CGDisplayStream provides a streaming API for capturing display updates in a realtime manner.  It can also provide
 scaling and color space conversion services, as well as allow capturing sub regions of the display.   Callbacks can be targetted
 at either a traditional CFRunLoop, or at a dispatch queue.
*/
typedef struct CF_BRIDGED_TYPE(id) CGDisplayStream *CGDisplayStreamRef;

/*!
 @typedef CGDisplayStreamUpdateRef
 @abstract An opaque reference to a single frame's extra metadata that describes useful frame delta information
 @discussion A CGDisplayStreamUpdate encapsulates information about what portions of a frame have changed relative to
 a previously delivered frame.   This includes regions that were changed in any way, which ones were actually redrawn, and which
 regions were merely copied from one place to another.   A routine is provided to merge two update refs together in cases
 where apps need to coalesce the values because they decided to skip processing for one or more frames.
*/
typedef const struct CF_BRIDGED_TYPE(id) CGDisplayStreamUpdate *CGDisplayStreamUpdateRef;

/*!
 @enum CGDisplayStreamUpdateRectType
 @abstract Used to select which array of rectangles to be returned by CGDisplayUpdateGetRects
 @const kCGDisplayStreamUpdateRefreshedRects The rectangles that were refreshed on the display, not counting moved rectangles
 @const kCGDisplayStreamUpdateMovedRects The rectangles that were simply moved from one part of the display to another
 @const kCGDisplayStreamUpdateDirtyRects The union of both refreshed and moved rects
 @const kCGDisplayStreamUpdateReducedDirtyRects A possibly simplified (but overstated) array of dirty rectangles
*/
typedef CF_ENUM(int32_t, CGDisplayStreamUpdateRectType) {
        kCGDisplayStreamUpdateRefreshedRects,
        kCGDisplayStreamUpdateMovedRects,
        kCGDisplayStreamUpdateDirtyRects,
        kCGDisplayStreamUpdateReducedDirtyRects,
};

/*!
 @enum CGDisplayStreamFrameStatus
 @abstract Provides information about incoming frame updates
 @const kCGDisplayStreamFrameStatusFrameComplete A new frame has been generated by the Window Server for a particular display at time displayTime
 @const kCGDisplayStreamFrameStatusFrameIdle The Window Server did not generate a new frame for displayTime
 @const kCGDisplayStreamFrameStatusFrameBlank As of displayTime, the display has gone blank
 @const kCGDisplayStreamFrameStatusStopped The display stream has stopped and no more calls will be made to the handler until the stream is started.
*/
typedef CF_ENUM(int32_t, CGDisplayStreamFrameStatus){
        kCGDisplayStreamFrameStatusFrameComplete,
        kCGDisplayStreamFrameStatusFrameIdle,
        kCGDisplayStreamFrameStatusFrameBlank,
        kCGDisplayStreamFrameStatusStopped,
};

/*
 @callback CGDisplayStreamFrameAvailableHandler
 @abstract The block prototype used for new frame delivery by CGDisplayStream objects
 @discussion For each frame that is generated by the WindowServer for a particular display, the user provided block is invoked and provides the user with an IOSurfaceRef
 that contains the pixel data for the new frame, as well as a CGDisplayStreamUpdateRef that contains all of the metadata associated with that IOSurface.
 @param frameSurface The IOSurfaceRef for the current frame.  May be NULL in some cases.   If you intend to hold on to the IOSurface beyond the lifetime of
 the handler call, you must CFRetain() the IOSurface until you are done with it *and* you must call IOSurfaceIncrementUseCount() to let the CGDisplayStream know
 that the frame is not ready for re-use.  Once you are finished using the IOSurfaceRef you must then call IOSurfaceDecrementUseCount().  If you are maintaing
 any kind of external cache of information about the IOSurface (such as a GL texture object), you must keep a CFRetain() on the IOSurface to prevent it from going
 away until you remove it from your cache.   You can not depend on the set of IOSurfaces being used by the display stream as being static, so you should implement
 some kind of age-out behavior for your cache for IOSurfaces that have not been re-used in a while.
 @param displayTime The mach_absolute_time() of when the corresponding frame was to be displayed by the WindowServer
 @param updateRef The CGDisplayStreamUpdateRef for the current frame.   Note: If you want to keep the CGDisplayStreamUpdateRef around beyond the lifetime
 of the handler, you must CFRetain() it, as it will be CFRelease()'d by the CGDisplayStream after the handler returns.   The updateRef will be NULL in cases
 when status is not kCGDisplayStreamFrameStatusFrameComplete.
 */
typedef void (^CGDisplayStreamFrameAvailableHandler)(CGDisplayStreamFrameStatus status, uint64_t displayTime, 
                                                                           IOSurfaceRef __nullable frameSurface,
                                                                           CGDisplayStreamUpdateRef __nullable updateRef);

/*!
 @function CGDisplayStreamUpdateGetTypeID
 @abstract Returns the CF "class" ID for CGDisplayStreamUpdate
 @result The CFTypeID of the CGDisplayStreamUpdate class.
*/
CG_EXTERN CFTypeID CGDisplayStreamUpdateGetTypeID(void)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamUpdateGetRects
 @abstract Returns a pointer to an array of CGRect structs that describe what parts of the frame have changed relative
 to the previously delivered frame.   This rectangle list encapsulates both the update rectangles and movement rectangles.
 @param updateRef The CGDisplayStreamUpdateRef
 @param rectCount A pointer to where the count of the number of rectangles in the array is to be returned. Must not be NULL.
 @result A pointer to the array of CGRectangles.  This array should not be freed by the caller.
*/
CG_EXTERN const CGRect * __nullable CGDisplayStreamUpdateGetRects(CGDisplayStreamUpdateRef __nullable updateRef, 
    CGDisplayStreamUpdateRectType rectType, size_t *  rectCount)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);
/*!
 @function CGDisplayStreamUpdateCreateMerged
 @abstract Merge two CGDisplayUpdateRefs into a new one.
 @discussion In cases where the client wishes to drop certain frame updates, this function may be used to merge two
 CGDisplayUpdateRefs together.  The core bit of functionality here is generating a new set of refresh/move/dirty
 rectangle arrays that properly represent the union of the deltas between the two frames.  Note that the ordering of
 the two refs is important.
  
 @param firstUpdate The first update (in a temporal sense)
 @param secondUpdate The second update (in a temporal sense)
 @result The new CGDisplayStreamUpdateRef 
*/
CG_EXTERN CGDisplayStreamUpdateRef __nullable CGDisplayStreamUpdateCreateMergedUpdate(
    CGDisplayStreamUpdateRef __nullable firstUpdate,
    CGDisplayStreamUpdateRef __nullable secondUpdate)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamUpdateGetMovedRectsDelta
 @abstract Return the movement dx and dy values for a single update
 @param updateRef The CGDisplayStreamUpdateRef
 @param dx A pointer to a CGFloat to store the x component of the movement delta
 @param dy A pointer to a CGFloat to store the y component of the movement delta
 @discussion The delta values describe the offset from the moved rectangles back to the source location.
*/
CG_EXTERN void CGDisplayStreamUpdateGetMovedRectsDelta(CGDisplayStreamUpdateRef __nullable updateRef,
    CGFloat *  dx, CGFloat *  dy)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamGetDropCount
 @abstract Return how many frames (if any) have been dropped since the last call to the handler.
 @param updateRef The CGDisplayStreamUpdateRef
 @result The number of dropped frames
 @discussion This call is primarily useful for performance measurement to determine if the client is keeping up with
 all WindowServer updates.
*/
CG_EXTERN size_t CGDisplayStreamUpdateGetDropCount(CGDisplayStreamUpdateRef __nullable updateRef)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/* Optional CGDisplayStream Properties */

/*!
 @const kCGDisplayStreamSourceRect
 @discussion This may be used to request a subregion of the display to be provided as the source of the display stream.  Use
 CGRectCreateDictionaryRepresentation to convert from a CGRect to the value used here.   Note: The coordinate system for the
 source rectangle is specified in display logical coordinates and not in pixels, in order to match the normal convention on
 HiDPI displays.
*/
CG_EXTERN const CFStringRef  kCGDisplayStreamSourceRect SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);  /* Source rectangle to capture - defaults to entire display */

/*!
 @const kCGDisplayStreamDestinationRect
 @discussion This may be used to request where within the destination buffer the display updates should be placed. Use
 CGRectCreateDictionaryRepresentation to convert from a CGRect to the value used here.   Note: The coordinate system for
 the destination rectangle is always specified in output pixels to match the fact that the output buffer size is also
 specified in terms of pixels.
 */
CG_EXTERN const CFStringRef  kCGDisplayStreamDestinationRect SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);     /* Destination rectangle - defaults to entire buffer */

/*!
 @const kCGDisplayStreamPreserveAspectRatio
 @discussion Enable/disable the work the Window Server will do to preserve the display aspect ratio.  By default the Window Server will
 assume that it should preserve the original aspect ratio of the source display rect.  If the aspect ratio of the source display and
 the display stream destination rect are not the same, black borders will be inserted at the top/bottom or right/left sides of the destination
 in order to preserve the source aspect ratio.
 */
CG_EXTERN const CFStringRef  kCGDisplayStreamPreserveAspectRatio SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0); /* CFBoolean - defaults to true */

/*!
 @const kCGDisplayStreamColorSpace
 @discussion Set the desired CGColorSpace of the output frames.  By default the color space will be that of the display.
*/
CG_EXTERN const CFStringRef  kCGDisplayStreamColorSpace SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0); /* Desired output color space (CGColorSpaceRef, macos(10.8,14.0)) - defaults to display color space */

/*!
 @const kCGDisplayStreamMinimumFrameTime
 @discussion Request that the delta between frame updates be at least as much specified by this value.
*/
CG_EXTERN const CFStringRef  kCGDisplayStreamMinimumFrameTime SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);    /* CFNumber in seconds, defaults to zero. */

/*!
 @const kCGDisplayStreamShowCursor
 @discussion Controls whether the cursor is embedded within the provided buffers or not.
*/
CG_EXTERN const CFStringRef  kCGDisplayStreamShowCursor SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);  /* CFBoolean - defaults to false */

/*!
 @const kCGDisplayStreamQueueDepth
 @discussion Controls how many frames deep the frame queue will be.  Defaults to N.
 */
CG_EXTERN const CFStringRef  kCGDisplayStreamQueueDepth SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);  /* Queue depth in frames.  Defaults to 3. */

/*!
 @const kCGDisplayStreamYCbCrMatrix
 @discussion When outputting frames in 420v or 420f format, this key may be used to control which YCbCr matrix is used
 The value should be one of the three kCGDisplayStreamYCbCrMatrix values specified below.
*/
CG_EXTERN const CFStringRef  kCGDisplayStreamYCbCrMatrix SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/* Supported YCbCr matrices. Note that these strings have identical values to the equivalent CoreVideo strings. */
CG_EXTERN const CFStringRef      kCGDisplayStreamYCbCrMatrix_ITU_R_709_2;
CG_EXTERN const CFStringRef      kCGDisplayStreamYCbCrMatrix_ITU_R_601_4;
CG_EXTERN const CFStringRef      kCGDisplayStreamYCbCrMatrix_SMPTE_240M_1995;

/*!
 @function CGDisplayStreamGetTypeID
 @abstract Returns the CF "class" ID for CGDisplayStream
 @result The CFTypeID of the CGDisplayStream class.
*/
CG_EXTERN CFTypeID CGDisplayStreamGetTypeID(void)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamCreate
 @abstract Creates a new CGDisplayStream intended to be used with a CFRunLoop
 @discussion This function creates a new CGDisplayStream that is to be used to get a stream of frame updates
 from a particular display.
 @param display The CGDirectDisplayID to use as the source for generated frames
 @param outputWidth The output width (in pixels, not points) of the frames to be generated.  Must not be zero.
 @param outputHeight The output height (in pixels, not points) of the frames to be generated.  Must not be zero.
 @param pixelFormat The desired CoreVideo/CoreMedia-style pixel format of the output IOSurfaces.  The currently
 supported values are:
 
 'BGRA' Packed Little Endian ARGB8888
 'l10r' Packed Little Endian ARGB2101010
 '420v' 2-plane "video" range YCbCr 4:2:0
 '420f' 2-plane "full" range YCbCr 4:2:0
 
 @param properties Any optional properties of the CGDisplayStream
 @param handler A block that will be called for frame deliver.
 @result The new CGDisplayStream object. 
*/
CG_EXTERN CGDisplayStreamRef __nullable CGDisplayStreamCreate(CGDirectDisplayID display, 
    size_t outputWidth, size_t outputHeight, int32_t pixelFormat, CFDictionaryRef __nullable properties,
    CGDisplayStreamFrameAvailableHandler __nullable handler)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamCreateWithDispatchQueue
 @abstract Creates a new CGDisplayStream intended to be serviced by a block handler
 @discussion This function creates a new CGDisplayStream that is to be used to get a stream of frame updates
 from a particular display.
 @param display The CGDirectDisplayID to use as the source for generated frames
 @param outputWidth The output width (in pixels, not points) of the frames to be generated.  Must not be zero.
 @param outputHeight The output height (in pixels, not points) of the frames to be generated.  Must not be zero.
 @param pixelFormat The desired CoreVideo/CoreMedia-style pixel format of the output IOSurfaces
 @param properties Any optional properties of the CGDisplayStream
 @param queue The dispatch_queue_t that will be used to invoke the callback handler.
 @param handler A block that will be called for frame deliver.
 @result The new CGDisplayStream object.
*/
CG_EXTERN CGDisplayStreamRef __nullable CGDisplayStreamCreateWithDispatchQueue(CGDirectDisplayID display, 
    size_t outputWidth, size_t outputHeight, int32_t pixelFormat, CFDictionaryRef __nullable properties,
    dispatch_queue_t  queue, CGDisplayStreamFrameAvailableHandler __nullable handler)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamStart
 @abstract Begin delivering frame updates to the handler block.
 @param displayStream to be started
 @result kCGErrorSuccess If the display stream was started, otherwise an error.
*/
CG_EXTERN CGError CGDisplayStreamStart(CGDisplayStreamRef cg_nullable displayStream)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamStop
 @abstract End delivery of frame updates to the handler block.
 @param displayStream to be stopped
 @result kCGErrorSuccess If the display stream was stopped, otherwise an error.
 @discussion After this call returns, the CGDisplayStream callback function will eventually be called with a
 status of kCGDisplayStreamFrameStatusStopped.  After that point it is safe to release the CGDisplayStream.
 It is safe to call this function from within the handler block, but the previous caveat still applies.
*/
CG_EXTERN CGError CGDisplayStreamStop(CGDisplayStreamRef cg_nullable displayStream)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

/*!
 @function CGDisplayStreamGetRunLoopSource
 @abstract Return the singleton CFRunLoopSourceRef for a CGDisplayStream.
 @param displayStream The CGDisplayStream object
 @result The CFRunLoopSource for this displayStream.  Note: This function will return NULL if the
 display stream was created via  CGDisplayStreamCreateWithDispatchQueue().
*/
CG_EXTERN CFRunLoopSourceRef __nullable CGDisplayStreamGetRunLoopSource(CGDisplayStreamRef cg_nullable displayStream)
    SCREEN_CAPTURE_OBSOLETE(10.8,14.0,15.0);

#endif /* __BLOCKS__ */

CF_ASSUME_NONNULL_END

CF_IMPLICIT_BRIDGING_DISABLED

#endif /* CGDISPLAYSTREAM_H_ */
