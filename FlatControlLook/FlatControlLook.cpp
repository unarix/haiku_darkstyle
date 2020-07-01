/*
 * Copyright 2019 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus, superstippi@gmx.de
 *		DarkWyrm, bpmagic@columbus.rr.com
 *		John Scipione, jscipione@gmail.com
 *		Clemens Zeidler, haiku@clemens-zeidler.de
 *		Nahuel Tello, ntello@unarix.com.ar
 */


/*! FlatControlLook flat Haiku */


#include "FlatControlLook.h"

#include <algorithm>
#include <cmath>
#include <new>
#include <stdio.h>

#include <WindowPrivate.h>

#include <Autolock.h>
#include <Debug.h>
#include <GradientLinear.h>
#include <Rect.h>
#include <Region.h>
#include <View.h>


//#define DEBUG_CONTROL_LOOK
#ifdef DEBUG_CONTROL_LOOK
#	define STRACE(x) printf x
#else
#	define STRACE(x) ;
#endif

static const float kEdgeBevelLightTint = 1.0;
static const float kEdgeBevelShadowTint = 1.0;
static const float kHoverTintFactor = 0.55;

static const float kButtonPopUpIndicatorWidth = 11;

FlatControlLook::FlatControlLook(image_id id)
	: HaikuControlLook()
{
}


FlatControlLook::~FlatControlLook()
{
}


void
FlatControlLook::DrawMenuBarBackground(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	uint32 borders)
{
	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	// the surface edges

	// colors
	float topTint;
	float bottomTint;

	if ((flags & B_ACTIVATED) != 0) {
		rgb_color bevelColor1 = tint_color(base, 1.0);
		rgb_color bevelColor2 = tint_color(base, 1.0);

		topTint = 1.0;
		bottomTint = 1.05;

		_DrawFrame(view, rect,
			bevelColor1, bevelColor1,
			bevelColor2, bevelColor2,
			borders & B_TOP_BORDER);
	} else {
		rgb_color cornerColor = tint_color(base, 1.0);
		rgb_color bevelColorTop = tint_color(base, 1.0);
		rgb_color bevelColorLeft = tint_color(base, 1.0);
		rgb_color bevelColorRightBottom = tint_color(base, 1.0);

		topTint = 1.0;
		bottomTint = 1.0;

		_DrawFrame(view, rect,
			bevelColorLeft, bevelColorTop,
			bevelColorRightBottom, bevelColorRightBottom,
			cornerColor, cornerColor,
			borders);
	}

	// draw surface top
	//_FillGradient(view, rect, base, bottomTint, topTint);
	_FillGradient(view, rect, base, topTint, bottomTint);
}


void
FlatControlLook::DrawMenuItemBackground(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	uint32 borders)
{
	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	// surface edges
	float topTint;
	float bottomTint;
	rgb_color selectedColor = base;

	if ((flags & B_ACTIVATED) != 0) {
		topTint = 1.0;
		bottomTint = 1.10;
	} else if ((flags & B_DISABLED) != 0) {
		topTint = 1.10;
		bottomTint = 1.20;
	} else {
		topTint = 1.0;
		bottomTint = 1.10;
	}

	rgb_color bevelLightColor = tint_color(selectedColor, topTint);
	rgb_color bevelShadowColor = tint_color(selectedColor, bottomTint);

	// draw surface edges
	_DrawFrame(view, rect,
		bevelShadowColor, bevelShadowColor,
		bevelShadowColor, bevelShadowColor,
		borders);

	// draw surface top
	view->SetLowColor(selectedColor);
	_FillGradient(view, rect, selectedColor, topTint, bottomTint);
}

void
FlatControlLook::DrawMenuFieldFrame(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base,
	const rgb_color& background, uint32 flags, uint32 borders)
{
	_DrawButtonFrame(view, rect, updateRect, 0.0f, 0.0f, 0.0f, 0.0f, base,
		background, 1.0, 1.0, flags, borders);
}


void
FlatControlLook::DrawMenuFieldFrame(BView* view, BRect& rect,
	const BRect& updateRect, float radius, const rgb_color& base,
	const rgb_color& background, uint32 flags, uint32 borders)
{
	_DrawButtonFrame(view, rect, updateRect, radius, radius, 0.0f, 0.0f,
		base, background, 1.0, 1.0, flags, borders);
}


void
FlatControlLook::DrawMenuFieldFrame(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius,
	float rightTopRadius, float leftBottomRadius,
	float rightBottomRadius, const rgb_color& base,
	const rgb_color& background, uint32 flags, uint32 borders)
{
	_DrawButtonFrame(view, rect, updateRect, leftTopRadius, rightTopRadius,
		0.0f, 0.0f, base, background, 1.0, 1.0,
		flags, borders);
}






void
HaikuControlLook::_DrawButtonFrame(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	const rgb_color& background, float contrast, float brightness,
	uint32 flags, uint32 borders)
{

        rgb_color customColor = tint_color(background, 1.0); // custom color for borders

        if (!rect.IsValid())
                return;

        // save the clipping constraints of the view
        view->PushState();

        // set clipping constraints to updateRect
        BRegion clipping(updateRect);
        view->ConstrainClippingRegion(&clipping);

        // If the button is flat and neither activated nor otherwise highlighted
        // (mouse hovering or focussed), draw it flat.
        if ((flags & B_FLAT) != 0
                && (flags & (B_ACTIVATED | B_PARTIALLY_ACTIVATED)) == 0
                && ((flags & (B_HOVER | B_FOCUSED)) == 0
                        || (flags & B_DISABLED) != 0)) {
                _DrawFrame(view, rect, customColor, customColor, customColor,
                        customColor, borders);
                _DrawFrame(view, rect, customColor, customColor, customColor,
                        customColor, borders);
                view->PopState();
                return;
        }

        // outer edge colors
        rgb_color edgeLightColor;
        rgb_color edgeShadowColor;

        // default button frame color
        rgb_color defaultIndicatorColor = customColor;
        rgb_color cornerBgColor;

        if ((flags & B_DISABLED) != 0) {
                defaultIndicatorColor = disable_color(defaultIndicatorColor,
                        customColor);
        }

        drawing_mode oldMode = view->DrawingMode();

        if ((flags & B_DEFAULT_BUTTON) != 0) {
                cornerBgColor = defaultIndicatorColor;
                edgeLightColor = _EdgeLightColor(defaultIndicatorColor,
                        contrast * ((flags & B_DISABLED) != 0 ? 0.0 : 0.0),
                        brightness * ((flags & B_DISABLED) != 0 ? 0.0 : 0.0), flags);
                edgeShadowColor = _EdgeShadowColor(defaultIndicatorColor,
                        contrast * ((flags & B_DISABLED) != 0 ? 0.0 : 0.0),
                        brightness * ((flags & B_DISABLED) != 0 ? 0.0 : 0.0), flags);

                // draw default button indicator
                // Allow a 1-pixel border of the background to come through.
                rect.InsetBy(1, 1);

                view->SetHighColor(defaultIndicatorColor);
                view->StrokeRoundRect(rect, leftTopRadius, leftTopRadius);
                rect.InsetBy(1, 1);

                view->StrokeRoundRect(rect, leftTopRadius, leftTopRadius);
                rect.InsetBy(1, 1);
        } else {
                cornerBgColor = customColor;
                if ((flags & B_BLEND_FRAME) != 0) {
                        // set the background color to transparent for the case
                        // that we are on the desktop
                        cornerBgColor.alpha = 0;
                        view->SetDrawingMode(B_OP_ALPHA);
                }

                edgeLightColor = _EdgeLightColor(customColor,
                        contrast * ((flags & B_DISABLED) != 0 ? 0.0 : 0.0),
                        brightness * 0.0, flags);
                edgeShadowColor = _EdgeShadowColor(customColor,
                        contrast * (flags & B_DISABLED) != 0 ? 0.0 : 0.0,
                        brightness * 0.0, flags);
        }

        // frame colors
        rgb_color frameLightColor  = _FrameLightColor(base, flags);
        rgb_color frameShadowColor = _FrameShadowColor(base, flags);

        // rounded corners

        if ((borders & B_LEFT_BORDER) != 0 && (borders & B_TOP_BORDER) != 0
                && leftTopRadius > 0) {
                // draw left top rounded corner
                BRect leftTopCorner(floorf(rect.left), floorf(rect.top),
                        floorf(rect.left + leftTopRadius),
                        floorf(rect.top + leftTopRadius));
                clipping.Exclude(leftTopCorner);
                _DrawRoundCornerFrameLeftTop(view, leftTopCorner, updateRect,
                        cornerBgColor, edgeShadowColor, frameLightColor);
        }

        if ((borders & B_TOP_BORDER) != 0 && (borders & B_RIGHT_BORDER) != 0
                && rightTopRadius > 0) {
                // draw right top rounded corner
                BRect rightTopCorner(floorf(rect.right - rightTopRadius),
                        floorf(rect.top), floorf(rect.right),
                        floorf(rect.top + rightTopRadius));
                clipping.Exclude(rightTopCorner);
                _DrawRoundCornerFrameRightTop(view, rightTopCorner, updateRect,
                        cornerBgColor, edgeShadowColor, edgeLightColor,
                        frameLightColor, frameShadowColor);
        }

        if ((borders & B_LEFT_BORDER) != 0 && (borders & B_BOTTOM_BORDER) != 0
                && leftBottomRadius > 0) {
                // draw left bottom rounded corner
                BRect leftBottomCorner(floorf(rect.left),
                        floorf(rect.bottom - leftBottomRadius),
                        floorf(rect.left + leftBottomRadius), floorf(rect.bottom));
                clipping.Exclude(leftBottomCorner);
                _DrawRoundCornerFrameLeftBottom(view, leftBottomCorner, updateRect,
                        cornerBgColor, edgeShadowColor, edgeLightColor,
                        frameLightColor, frameShadowColor);
        }

        if ((borders & B_RIGHT_BORDER) != 0 && (borders & B_BOTTOM_BORDER) != 0
                && rightBottomRadius > 0) {
                // draw right bottom rounded corner
                BRect rightBottomCorner(floorf(rect.right - rightBottomRadius),
                        floorf(rect.bottom - rightBottomRadius), floorf(rect.right),
                        floorf(rect.bottom));
                clipping.Exclude(rightBottomCorner);
                _DrawRoundCornerFrameRightBottom(view, rightBottomCorner,
                        updateRect, cornerBgColor, edgeLightColor, frameShadowColor);
        }

        // clip out the corners
        view->ConstrainClippingRegion(&clipping);

		// draw outer edge
        if ((flags & B_DEFAULT_BUTTON) != 0) {
                _DrawOuterResessedFrame(view, rect, defaultIndicatorColor,
                        contrast * ((flags & B_DISABLED) != 0 ? 0.3 : 0.8),
                        brightness * ((flags & B_DISABLED) != 0 ? 0.3 : 0.9),
                        flags, borders);
        } else {
                _DrawOuterResessedFrame(view, rect, customColor,
                        contrast * ((flags & B_DISABLED) != 0 ? 0.3 : 1.0),
                        brightness * 1.0, flags, borders);
        }

        view->SetDrawingMode(oldMode);

		rgb_color customColor2 = tint_color(background, 1.14); // custom color for borders

        // draw frame
        if ((flags & B_BLEND_FRAME) != 0) {
                drawing_mode oldDrawingMode = view->DrawingMode();
                view->SetDrawingMode(B_OP_ALPHA);

                _DrawFrame(view, rect, customColor2, customColor2,
                        customColor2, customColor2, borders);

                view->SetDrawingMode(oldDrawingMode);
        } else {
                _DrawFrame(view, rect, customColor2, customColor2,
                        customColor2, customColor2, borders);
        }

        // restore the clipping constraints of the view
        view->PopState();
}

void
FlatControlLook::DrawButtonBackground(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	uint32 borders, orientation orientation)
{
	_DrawButtonBackground(view, rect, updateRect, 0.0f, 0.0f, 0.0f, 0.0f,
		base, false, flags, borders, orientation);
}


void
FlatControlLook::DrawButtonBackground(BView* view, BRect& rect,
	const BRect& updateRect, float radius, const rgb_color& base, uint32 flags,
	uint32 borders, orientation orientation)
{
	_DrawButtonBackground(view, rect, updateRect, radius, radius, radius,
		radius, base, false, flags, borders, orientation);
}


void
FlatControlLook::DrawButtonBackground(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	uint32 flags, uint32 borders, orientation orientation)
{
	_DrawButtonBackground(view, rect, updateRect, leftTopRadius,
		rightTopRadius, leftBottomRadius, rightBottomRadius, base, false, flags,
		borders, orientation);
}


void
FlatControlLook::_DrawButtonBackground(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	bool popupIndicator, uint32 flags, uint32 borders, orientation orientation)
{
	//(rgb_color){200,165,35,255};

	if (!rect.IsValid())
		return;

	// save the clipping constraints of the view
	view->PushState();

	// set clipping constraints to updateRect
	BRegion clipping(updateRect);
	view->ConstrainClippingRegion(&clipping);

	// If the button is flat and neither activated nor otherwise highlighted
	// (mouse hovering or focussed), draw it flat.
	if ((flags & B_FLAT) != 0
		&& (flags & (B_ACTIVATED | B_PARTIALLY_ACTIVATED)) == 0
		&& ((flags & (B_HOVER | B_FOCUSED)) == 0
			|| (flags & B_DISABLED) != 0)) {
		_DrawFlatButtonBackground(view, rect, updateRect, base, popupIndicator,
			flags, borders, orientation);
	} else {
		_DrawNonFlatButtonBackground(view, rect, updateRect, clipping,
			leftTopRadius, rightTopRadius, leftBottomRadius, rightBottomRadius,
			base, popupIndicator, flags, borders, orientation);
	}

	// restore the clipping constraints of the view
	view->PopState();
}



void
FlatControlLook::_DrawNonFlatButtonBackground(BView* view, BRect& rect,
	const BRect& updateRect, BRegion& clipping, float leftTopRadius,
	float rightTopRadius, float leftBottomRadius, float rightBottomRadius,
	const rgb_color& base, bool popupIndicator, uint32 flags, uint32 borders,
	orientation orientation)
{
	// inner bevel colors
	rgb_color bevelLightColor  = _BevelLightColor(base, flags);
	rgb_color bevelShadowColor = _BevelShadowColor(base, flags);

	// button background color
	rgb_color buttonBgColor;
	if ((flags & B_DISABLED) != 0)
		buttonBgColor = tint_color(base, 1.0);
	else
		buttonBgColor = tint_color(base, 1.0);

	// surface top gradient
	BGradientLinear fillGradient;
	_MakeButtonGradient(fillGradient, rect, base, flags, orientation);

	// rounded corners

	if ((borders & B_LEFT_BORDER) != 0 && (borders & B_TOP_BORDER) != 0
		&& leftTopRadius > 0) {
		// draw left top rounded corner
		BRect leftTopCorner(floorf(rect.left), floorf(rect.top),
			floorf(rect.left + leftTopRadius - 2.0),
			floorf(rect.top + leftTopRadius - 2.0));
		clipping.Exclude(leftTopCorner);
		_DrawRoundCornerBackgroundLeftTop(view, leftTopCorner, updateRect,
			bevelLightColor, fillGradient);
	}

	if ((borders & B_TOP_BORDER) != 0 && (borders & B_RIGHT_BORDER) != 0
		&& rightTopRadius > 0) {
		// draw right top rounded corner
		BRect rightTopCorner(floorf(rect.right - rightTopRadius + 2.0),
			floorf(rect.top), floorf(rect.right),
			floorf(rect.top + rightTopRadius - 2.0));
		clipping.Exclude(rightTopCorner);
		_DrawRoundCornerBackgroundRightTop(view, rightTopCorner,
			updateRect, bevelLightColor, bevelShadowColor, fillGradient);
	}

	if ((borders & B_LEFT_BORDER) != 0 && (borders & B_BOTTOM_BORDER) != 0
		&& leftBottomRadius > 0) {
		// draw left bottom rounded corner
		BRect leftBottomCorner(floorf(rect.left),
			floorf(rect.bottom - leftBottomRadius + 2.0),
			floorf(rect.left + leftBottomRadius - 2.0),
			floorf(rect.bottom));
		clipping.Exclude(leftBottomCorner);
		_DrawRoundCornerBackgroundLeftBottom(view, leftBottomCorner,
			updateRect, bevelLightColor, bevelShadowColor, fillGradient);
	}

	if ((borders & B_RIGHT_BORDER) != 0 && (borders & B_BOTTOM_BORDER) != 0
		&& rightBottomRadius > 0) {
		// draw right bottom rounded corner
		BRect rightBottomCorner(floorf(rect.right - rightBottomRadius + 2.0),
			floorf(rect.bottom - rightBottomRadius + 2.0), floorf(rect.right),
			floorf(rect.bottom));
		clipping.Exclude(rightBottomCorner);
		_DrawRoundCornerBackgroundRightBottom(view, rightBottomCorner,
			updateRect, bevelShadowColor, fillGradient);
	}

	// clip out the corners
	view->ConstrainClippingRegion(&clipping);

	// draw inner bevel

	if ((flags & B_ACTIVATED) != 0) {
		view->BeginLineArray(4);

		// shadow along left/top borders
		if (borders & B_LEFT_BORDER) {
			view->AddLine(BPoint(rect.left, rect.top),
				BPoint(rect.left, rect.bottom), buttonBgColor);
			rect.left++;
		}
		if (borders & B_TOP_BORDER) {
			view->AddLine(BPoint(rect.left, rect.top),
				BPoint(rect.right, rect.top), buttonBgColor);
			rect.top++;
		}

		// softer shadow along left/top borders
		if (borders & B_LEFT_BORDER) {
			view->AddLine(BPoint(rect.left, rect.top),
				BPoint(rect.left, rect.bottom), buttonBgColor);
			rect.left++;
		}
		if (borders & B_TOP_BORDER) {
			view->AddLine(BPoint(rect.left, rect.top),
				BPoint(rect.right, rect.top), buttonBgColor);
			rect.top++;
		}

		view->EndLineArray();
	} else {
		_DrawFrame(view, rect,
			buttonBgColor, buttonBgColor,
			buttonBgColor, buttonBgColor,
			buttonBgColor, buttonBgColor, borders);
	}

	if (popupIndicator) {
		BRect indicatorRect(rect);
		rect.right -= kButtonPopUpIndicatorWidth;
		indicatorRect.left = rect.right + 3;
			// 2 pixels for the separator

		// Even when depressed we want the pop-up indicator background and
		// separator to cover the area up to the top.
		if ((flags & B_ACTIVATED) != 0)
			indicatorRect.top--;

		// draw the separator
		rgb_color separatorBaseColor = base;
		if ((flags & B_ACTIVATED) != 0)
			separatorBaseColor = tint_color(base, B_DARKEN_1_TINT);

		rgb_color separatorLightColor = _EdgeShadowColor(separatorBaseColor,
			(flags & B_DISABLED) != 0 ? 1.7 : 1.7, 2.0, flags);
		rgb_color separatorShadowColor = _EdgeShadowColor(separatorBaseColor,
			(flags & B_DISABLED) != 0 ? 1.7 : 1.7, 2.0, flags);

		view->BeginLineArray(2);

		view->AddLine(BPoint(indicatorRect.left - 2, indicatorRect.top),
			BPoint(indicatorRect.left - 2, indicatorRect.bottom),
			separatorShadowColor);
		view->AddLine(BPoint(indicatorRect.left - 1, indicatorRect.top),
			BPoint(indicatorRect.left - 1, indicatorRect.bottom),
			separatorShadowColor);

		view->EndLineArray();

		// draw background and pop-up marker
		_DrawMenuFieldBackgroundInside(view, indicatorRect, updateRect,
			0.0f, rightTopRadius, 0.0f, rightBottomRadius, base, flags, 0);

		if ((flags & B_ACTIVATED) != 0)
			indicatorRect.top++;

		_DrawPopUpMarker(view, indicatorRect, base, flags);
	}

	// fill in the background
	view->FillRect(rect, fillGradient);
}




void
HaikuControlLook::_DrawPopUpMarker(BView* view, const BRect& rect,
	const rgb_color& base, uint32 flags)
{
	BPoint center(roundf((rect.left + rect.right) / 2.0),
		roundf((rect.top + rect.bottom) / 2.0));
	BPoint triangle[3];
	triangle[0] = center + BPoint(-2.5, -0.5);
	triangle[1] = center + BPoint(2.5, -0.5);
	triangle[2] = center + BPoint(0.0, 2.0);

	uint32 viewFlags = view->Flags();
	view->SetFlags(viewFlags | B_SUBPIXEL_PRECISE);

	rgb_color markColor;
	if ((flags & B_DISABLED) != 0)
		markColor = tint_color(base, 1.0);
	else
		markColor = tint_color(base, 1.70);

	view->SetHighColor(markColor);
	view->FillTriangle(triangle[0], triangle[1], triangle[2]);

	view->SetFlags(viewFlags);
}

void
HaikuControlLook::_MakeButtonGradient(BGradientLinear& gradient, BRect& rect,
	const rgb_color& base, uint32 flags, orientation orientation) const
{
	float topTint = 0.99;
	float middleTint1 = 0.99;
	float middleTint2 = 1.0;
	float bottomTint = 1.04;

	if ((flags & B_ACTIVATED) != 0) {
		topTint = 1.11;
		bottomTint = 1.08;
	}

	if ((flags & B_DISABLED) != 0) {
		topTint = (topTint + B_NO_TINT) / 2;
		middleTint1 = (middleTint1 + B_NO_TINT) / 2;
		middleTint2 = (middleTint2 + B_NO_TINT) / 2;
		bottomTint = (bottomTint + B_NO_TINT) / 2;
	} else if ((flags & B_HOVER) != 0) {
		topTint = 1.0;
		middleTint1 = 1.0;
		middleTint2 = 1.0;
		bottomTint = 1.0;
	}

	if ((flags & B_ACTIVATED) != 0) {
		_MakeGradient(gradient, rect, base, topTint, bottomTint, orientation);
	} else {
		_MakeGlossyGradient(gradient, rect, base, topTint, middleTint1,
			middleTint2, bottomTint, orientation);
	}
}

void
HaikuControlLook::_MakeGlossyGradient(BGradientLinear& gradient, const BRect& rect,
	const rgb_color& base, float topTint, float middle1Tint,
	float middle2Tint, float bottomTint,
	orientation orientation) const
{
	gradient.AddColor(tint_color(base, topTint), 0);
	//gradient.AddColor(tint_color(base, middle1Tint), 150);
	//gradient.AddColor(tint_color(base, middle2Tint), 200);
	gradient.AddColor(tint_color(base, bottomTint), 255);
	gradient.SetStart(rect.LeftTop());
	if (orientation == B_HORIZONTAL)
		gradient.SetEnd(rect.LeftBottom());
	else
		gradient.SetEnd(rect.RightTop());
}

void
HaikuControlLook::_MakeGradient(BGradientLinear& gradient, const BRect& rect,
	const rgb_color& base, float topTint, float bottomTint,
	orientation orientation) const
{
	gradient.AddColor(tint_color(base, 0.97), 0);
	gradient.AddColor(tint_color(base, 1.0), 255);
	gradient.SetStart(rect.LeftTop());
	if (orientation == B_HORIZONTAL)
		gradient.SetEnd(rect.LeftBottom());
	else
		gradient.SetEnd(rect.RightTop());
}

void
HaikuControlLook::DrawScrollBarButton(BView* view, BRect rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	int32 direction, orientation orientation, bool down)
{
	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	// clip to button
	BRegion buttonRegion(rect);
	view->ConstrainClippingRegion(&buttonRegion);

	bool isEnabled = (flags & B_DISABLED) == 0;

	DrawButtonBackground(view, rect, updateRect, base, flags,
		BControlLook::B_ALL_BORDERS, orientation);

	rect.InsetBy(-1, -1);
	DrawArrowShape(view, rect, updateRect, base, direction, flags, 1.9f);
		// almost but not quite B_DARKEN_MAX_TINT

	// revert clipping constraints
	BRegion clipping(updateRect);
	view->ConstrainClippingRegion(&clipping);
}

void
FlatControlLook::DrawScrollBarThumb(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	orientation orientation, uint32 knobStyle)
{
        if (!rect.IsValid() || !rect.Intersects(updateRect))
                return;

        view->PushState();

        // set clipping constraints to updateRect
        BRegion clipping(updateRect);
        view->ConstrainClippingRegion(&clipping);

        // flags
        bool isEnabled = (flags & B_DISABLED) == 0;

        // colors
        rgb_color thumbColor = ui_color(B_SCROLL_BAR_THUMB_COLOR);
        const float bgTint = 1.06;

        rgb_color light, dark, dark1, dark2;
        light = tint_color(base, B_LIGHTEN_MAX_TINT);
        dark = tint_color(base, B_DARKEN_3_TINT);
        dark1 = tint_color(base, B_DARKEN_1_TINT);
        dark2 = tint_color(base, B_DARKEN_2_TINT);

        // draw thumb over background
        view->SetDrawingMode(B_OP_OVER);
        view->SetHighColor(dark1);

        // draw scroll thumb
        if (isEnabled) {
                // fill the clickable surface of the thumb
                DrawButtonBackground(view, rect, updateRect, thumbColor, 0,
                        B_ALL_BORDERS, orientation);
        } else {
                // thumb bevel
                view->BeginLineArray(4);
                view->AddLine(BPoint(rect.left, rect.bottom),
                        BPoint(rect.left, rect.top), dark1);
                view->AddLine(BPoint(rect.left + 1, rect.top),
                        BPoint(rect.right, rect.top), dark1);
                view->AddLine(BPoint(rect.right, rect.top + 1),
                        BPoint(rect.right, rect.bottom), dark1);
                view->AddLine(BPoint(rect.right - 1, rect.bottom),
                        BPoint(rect.left + 1, rect.bottom), dark1);
                view->EndLineArray();

                // thumb fill
                rect.InsetBy(1, 1);
                view->SetHighColor(dark1);
                view->FillRect(rect);
        }

        knobStyle = B_KNOB_LINES;

        // draw knob style
        if (knobStyle != B_KNOB_NONE) {
                rgb_color knobLight = isEnabled
                        ? tint_color(thumbColor, 1.0)
                        : tint_color(dark1, bgTint);
                rgb_color knobDark = isEnabled
                        ? tint_color(thumbColor, 1.2)
                        : tint_color(knobLight, 1.2);

                if (knobStyle == B_KNOB_DOTS) {
                        // draw dots on the scroll bar thumb
                        float hcenter = rect.left + rect.Width() / 2;
                        float vmiddle = rect.top + rect.Height() / 2;
                        BRect knob(hcenter, vmiddle, hcenter, vmiddle);

                        if (orientation == B_HORIZONTAL) {
                                view->SetHighColor(knobDark);
                                view->FillRect(knob);
                                view->SetHighColor(knobLight);
                                view->FillRect(knob.OffsetByCopy(1, 1));

                                float spacer = rect.Height();

                                if (rect.left + 3 < hcenter - spacer) {
                                        view->SetHighColor(knobDark);
                                        view->FillRect(knob.OffsetByCopy(-spacer, 0));
                                        view->SetHighColor(knobLight);
                                        view->FillRect(knob.OffsetByCopy(-spacer + 1, 1));
                                }

                                if (rect.right - 3 > hcenter + spacer) {
                                        view->SetHighColor(knobDark);
                                        view->FillRect(knob.OffsetByCopy(spacer, 0));
                                        view->SetHighColor(knobLight);
                                        view->FillRect(knob.OffsetByCopy(spacer + 1, 1));
                                }
                        } else {
                                // B_VERTICAL
                                view->SetHighColor(knobDark);
                                view->FillRect(knob);
                                view->SetHighColor(knobLight);
                                view->FillRect(knob.OffsetByCopy(1, 1));

                                float spacer = rect.Width();

                                if (rect.top + 3 < vmiddle - spacer) {
                                        view->SetHighColor(knobDark);
                                        view->FillRect(knob.OffsetByCopy(0, -spacer));
                                        view->SetHighColor(knobLight);
                                        view->FillRect(knob.OffsetByCopy(1, -spacer + 1));
                                }

                                if (rect.bottom - 3 > vmiddle + spacer) {
                                        view->SetHighColor(knobDark);
                                        view->FillRect(knob.OffsetByCopy(0, spacer));
                                        view->SetHighColor(knobLight);
                                        view->FillRect(knob.OffsetByCopy(1, spacer + 1));
                                }
                        }
                } else if (knobStyle == B_KNOB_LINES && isEnabled) {
                        // draw lines on the scroll bar thumb
                        if (orientation == B_HORIZONTAL) {
                                float middle = rect.Width() / 2;

                                view->BeginLineArray(6);
                                view->AddLine(
                                        BPoint(rect.left + middle - 3, rect.top + 2),
                                        BPoint(rect.left + middle - 3, rect.bottom - 2),
                                        knobDark);
                                view->AddLine(
                                        BPoint(rect.left + middle, rect.top + 2),
                                        BPoint(rect.left + middle, rect.bottom - 2),
                                        knobDark);
                                view->AddLine(
                                        BPoint(rect.left + middle + 3, rect.top + 2),
                                        BPoint(rect.left + middle + 3, rect.bottom - 2),
                                        knobDark);
                                view->AddLine(
                                        BPoint(rect.left + middle - 2, rect.top + 2),
                                        BPoint(rect.left + middle - 2, rect.bottom - 2),
                                        knobLight);
                                view->AddLine(
                                        BPoint(rect.left + middle + 1, rect.top + 2),
                                        BPoint(rect.left + middle + 1, rect.bottom - 2),
                                        knobLight);
                                view->AddLine(
                                        BPoint(rect.left + middle + 4, rect.top + 2),
                                        BPoint(rect.left + middle + 4, rect.bottom - 2),
                                        knobLight);
                                view->EndLineArray();
                        } else {
                                // B_VERTICAL
                                float middle = rect.Height() / 2;

                                view->BeginLineArray(6);
                                view->AddLine(
                                        BPoint(rect.left + 2, rect.top + middle - 3),
                                        BPoint(rect.right - 2, rect.top + middle - 3),
                                        knobDark);
                                view->AddLine(
                                        BPoint(rect.left + 2, rect.top + middle),
                                        BPoint(rect.right - 2, rect.top + middle),
                                        knobDark);
                                view->AddLine(
                                        BPoint(rect.left + 2, rect.top + middle + 3),
                                        BPoint(rect.right - 2, rect.top + middle + 3),
                                        knobDark);
                                view->AddLine(
                                        BPoint(rect.left + 2, rect.top + middle - 2),
                                        BPoint(rect.right - 2, rect.top + middle - 2),
                                        knobLight);
                                view->AddLine(
                                        BPoint(rect.left + 2, rect.top + middle + 1),
                                        BPoint(rect.right - 2, rect.top + middle + 1),
                                        knobLight);
                                view->AddLine(
                                        BPoint(rect.left + 2, rect.top + middle + 4),
                                        BPoint(rect.right - 2, rect.top + middle + 4),
                                        knobLight);
                                view->EndLineArray();
                        }
                }
        }

        view->PopState();
}

void
HaikuControlLook::DrawGroupFrame(BView* view, BRect& rect, const BRect& updateRect,
	const rgb_color& base, uint32 borders)
{
	rgb_color frameColor = tint_color(base, 1.05);
	rgb_color bevelLight = tint_color(base, 1.0);
	rgb_color bevelShadow = tint_color(base, 1.0);

	_DrawFrame(view, rect, bevelShadow, bevelShadow, bevelLight, bevelLight,
		borders);

	_DrawFrame(view, rect, frameColor, frameColor, frameColor, frameColor,
		borders);

	_DrawFrame(view, rect, bevelLight, bevelLight, bevelShadow, bevelShadow,
		borders);
}

void
HaikuControlLook::DrawActiveTab(BView* view, BRect& rect, const BRect& updateRect,
	const rgb_color& base, uint32 flags, uint32 borders, uint32 side)
{
	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	// Snap the rectangle to pixels to avoid rounding errors.
	rect.left = floorf(rect.left);
	rect.right = floorf(rect.right);
	rect.top = floorf(rect.top);
	rect.bottom = floorf(rect.bottom);

	// save the clipping constraints of the view
	view->PushState();

	// set clipping constraints to updateRect
	BRegion clipping(updateRect);
	view->ConstrainClippingRegion(&clipping);

	rgb_color edgeShadowColor;
	rgb_color edgeLightColor;
	rgb_color frameShadowColor;
	rgb_color frameLightColor;
	rgb_color bevelShadowColor;
	rgb_color bevelLightColor;
	BGradientLinear fillGradient;
	fillGradient.SetStart(rect.LeftTop() + BPoint(3, 3));
	fillGradient.SetEnd(rect.LeftBottom() + BPoint(3, -3));

	if ((flags & B_DISABLED) != 0) {
		edgeLightColor = base;
		edgeShadowColor = base;
		frameLightColor = tint_color(base, 1.0);
		frameShadowColor = tint_color(base, 1.30);
		bevelLightColor = tint_color(base, 0.8);
		bevelShadowColor = tint_color(base, 1.07);
		fillGradient.AddColor(tint_color(base, 0.85), 0);
		fillGradient.AddColor(base, 255);
	} else {
		edgeLightColor = tint_color(base, 0.95);
		edgeShadowColor = tint_color(base, 1.03);
		frameLightColor = tint_color(base, 1.30);
		frameShadowColor = tint_color(base, 1.30);
		bevelLightColor = tint_color(base, 0.9);
		bevelShadowColor = tint_color(base, 1.07);
		fillGradient.AddColor(tint_color(base, 0.95), 0);
		fillGradient.AddColor(tint_color(base, 1.0), 255);
	}

	static const float kRoundCornerRadius = 2.0f;

	// left top corner dimensions
	BRect leftTopCorner(rect);
	leftTopCorner.right = floorf(leftTopCorner.left + kRoundCornerRadius);
	leftTopCorner.bottom = floorf(rect.top + kRoundCornerRadius);

	// right top corner dimensions
	BRect rightTopCorner(rect);
	rightTopCorner.left = floorf(rightTopCorner.right - kRoundCornerRadius);
	rightTopCorner.bottom = floorf(rect.top + kRoundCornerRadius);

	// left bottom corner dimensions
	BRect leftBottomCorner(rect);
	leftBottomCorner.right = floorf(leftBottomCorner.left + kRoundCornerRadius);
	leftBottomCorner.top = floorf(rect.bottom - kRoundCornerRadius);

	// right bottom corner dimensions
	BRect rightBottomCorner(rect);
	rightBottomCorner.left = floorf(rightBottomCorner.right
		- kRoundCornerRadius);
	rightBottomCorner.top = floorf(rect.bottom - kRoundCornerRadius);

	switch (side) {
		case B_TOP_BORDER:
			clipping.Exclude(leftTopCorner);
			clipping.Exclude(rightTopCorner);

			// draw the left top corner
			_DrawRoundCornerLeftTop(view, leftTopCorner, updateRect, base,
				edgeShadowColor, frameLightColor, bevelLightColor,
				fillGradient);
			// draw the right top corner
			_DrawRoundCornerRightTop(view, rightTopCorner, updateRect, base,
				edgeShadowColor, edgeLightColor, frameLightColor,
				frameShadowColor, bevelLightColor, bevelShadowColor,
				fillGradient);
			break;
		case B_BOTTOM_BORDER:
			clipping.Exclude(leftBottomCorner);
			clipping.Exclude(rightBottomCorner);

			// draw the left bottom corner
			_DrawRoundCornerLeftBottom(view, leftBottomCorner, updateRect, base,
				edgeShadowColor, edgeLightColor, frameLightColor,
				frameShadowColor, bevelLightColor, bevelShadowColor,
				fillGradient);
			// draw the right bottom corner
			_DrawRoundCornerRightBottom(view, rightBottomCorner, updateRect,
				base, edgeLightColor, frameShadowColor, bevelShadowColor,
				fillGradient);
			break;
		case B_LEFT_BORDER:
			clipping.Exclude(leftTopCorner);
			clipping.Exclude(leftBottomCorner);

			// draw the left top corner
			_DrawRoundCornerLeftTop(view, leftTopCorner, updateRect, base,
				edgeShadowColor, frameLightColor, bevelLightColor,
				fillGradient);
			// draw the left bottom corner
			_DrawRoundCornerLeftBottom(view, leftBottomCorner, updateRect, base,
				edgeShadowColor, edgeLightColor, frameLightColor,
				frameShadowColor, bevelLightColor, bevelShadowColor,
				fillGradient);
			break;
		case B_RIGHT_BORDER:
			clipping.Exclude(rightTopCorner);
			clipping.Exclude(rightBottomCorner);

			// draw the right top corner
			_DrawRoundCornerRightTop(view, rightTopCorner, updateRect, base,
				edgeShadowColor, edgeLightColor, frameLightColor,
				frameShadowColor, bevelLightColor, bevelShadowColor,
				fillGradient);
			// draw the right bottom corner
			_DrawRoundCornerRightBottom(view, rightBottomCorner, updateRect,
				base, edgeLightColor, frameShadowColor, bevelShadowColor,
				fillGradient);
			break;
	}

	// clip out the corners
	view->ConstrainClippingRegion(&clipping);

	uint32 bordersToDraw = 0;
	switch (side) {
		case B_TOP_BORDER:
			bordersToDraw = (B_LEFT_BORDER | B_TOP_BORDER | B_RIGHT_BORDER);
			break;
		case B_BOTTOM_BORDER:
			bordersToDraw = (B_LEFT_BORDER | B_BOTTOM_BORDER | B_RIGHT_BORDER);
			break;
		case B_LEFT_BORDER:
			bordersToDraw = (B_LEFT_BORDER | B_BOTTOM_BORDER | B_TOP_BORDER);
			break;
		case B_RIGHT_BORDER:
			bordersToDraw = (B_RIGHT_BORDER | B_BOTTOM_BORDER | B_TOP_BORDER);
			break;
	}

	// draw the rest of frame and fill
	_DrawFrame(view, rect, edgeShadowColor, edgeShadowColor, edgeLightColor,
		edgeLightColor, borders);
	if (side == B_TOP_BORDER || side == B_BOTTOM_BORDER) {
		if ((borders & B_LEFT_BORDER) == 0)
			rect.left++;
		if ((borders & B_RIGHT_BORDER) == 0)
			rect.right--;
	} else if (side == B_LEFT_BORDER || side == B_RIGHT_BORDER) {
		if ((borders & B_TOP_BORDER) == 0)
			rect.top++;
		if ((borders & B_BOTTOM_BORDER) == 0)
			rect.bottom--;
	}

	_DrawFrame(view, rect, frameLightColor, frameLightColor, frameShadowColor,
		frameShadowColor, bordersToDraw);

	_DrawFrame(view, rect, bevelLightColor, bevelLightColor, bevelShadowColor,
		bevelShadowColor);

	view->FillRect(rect, fillGradient);

	// restore the clipping constraints of the view
	view->PopState();
}

void
FlatControlLook::DrawSplitter(BView* view, BRect& rect, const BRect& updateRect,
	const rgb_color& base, orientation orientation, uint32 flags,
	uint32 borders)
{
	return;

	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	rgb_color background;
	if ((flags & (B_CLICKED | B_ACTIVATED)) != 0)
		background = tint_color(base, B_DARKEN_1_TINT);
	else
		background = base;

	rgb_color light = tint_color(background, 1.0);
	rgb_color shadow = tint_color(background, 1.0);

	// frame
	if (borders != 0 && rect.Width() > 3 && rect.Height() > 3)
		DrawRaisedBorder(view, rect, updateRect, background, flags, borders);

	// dots and rest of background
	if (orientation == B_HORIZONTAL) {
		if (rect.Width() > 2) {
			// background on left/right
			BRegion region(rect);
			rect.left = floorf((rect.left + rect.right) / 2.0 - 0.5);
			rect.right = rect.left + 1;
			region.Exclude(rect);
			view->SetHighColor(background);
			view->FillRegion(&region);
		}

		BPoint dot = rect.LeftTop();
		BPoint stop = rect.LeftBottom();
		int32 num = 1;
		while (dot.y <= stop.y) {
			rgb_color col1;
			rgb_color col2;
			switch (num) {
				case 1:
					col1 = background;
					col2 = background;
					break;
				case 2:
					col1 = shadow;
					col2 = background;
					break;
				case 3:
				default:
					col1 = background;
					col2 = light;
					num = 0;
					break;
			}
			view->SetHighColor(col1);
			view->StrokeLine(dot, dot, B_SOLID_LOW);
			view->SetHighColor(col2);
			dot.x++;
			view->StrokeLine(dot, dot, B_SOLID_LOW);
			dot.x -= 1.0;
			// next pixel
			num++;
			dot.y++;
		}
	} else {
		if (rect.Height() > 2) {
			// background on left/right
			BRegion region(rect);
			rect.top = floorf((rect.top + rect.bottom) / 2.0 - 0.5);
			rect.bottom = rect.top + 1;
			region.Exclude(rect);
			view->SetHighColor(background);
			view->FillRegion(&region);
		}

		BPoint dot = rect.LeftTop();
		BPoint stop = rect.RightTop();
		int32 num = 1;
		while (dot.x <= stop.x) {
			rgb_color col1;
			rgb_color col2;
			switch (num) {
				case 1:
					col1 = background;
					col2 = background;
					break;
				case 2:
					col1 = shadow;
					col2 = background;
					break;
				case 3:
				default:
					col1 = background;
					col2 = light;
					num = 0;
					break;
			}
			view->SetHighColor(col1);
			view->StrokeLine(dot, dot, B_SOLID_LOW);
			view->SetHighColor(col2);
			dot.y++;
			view->StrokeLine(dot, dot, B_SOLID_LOW);
			dot.y -= 1.0;
			// next pixel
			num++;
			dot.x++;
		}
	}
}

void
FlatControlLook::DrawButtonWithPopUpBackground(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	uint32 borders, orientation orientation)
{
	_DrawButtonBackground(view, rect, updateRect, 0.0f, 0.0f, 0.0f, 0.0f,
		base, true, flags, borders, orientation);
}


void
FlatControlLook::DrawButtonWithPopUpBackground(BView* view, BRect& rect,
	const BRect& updateRect, float radius, const rgb_color& base, uint32 flags,
	uint32 borders, orientation orientation)
{
	_DrawButtonBackground(view, rect, updateRect, radius, radius, radius,
		radius, base, true, flags, borders, orientation);
}


void
FlatControlLook::DrawButtonWithPopUpBackground(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	uint32 flags, uint32 borders, orientation orientation)
{
	_DrawButtonBackground(view, rect, updateRect, leftTopRadius,
		rightTopRadius, leftBottomRadius, rightBottomRadius, base, true, flags,
		borders, orientation);
}


void
HaikuControlLook::DrawButtonFrame(BView* view, BRect& rect, const BRect& updateRect,
	const rgb_color& base, const rgb_color& background, uint32 flags,
	uint32 borders)
{
	_DrawButtonFrame(view, rect, updateRect, 0.0f, 0.0f, 0.0f, 0.0f, base,
		background, 1.0, 1.0, flags, borders);
}


void
HaikuControlLook::DrawButtonFrame(BView* view, BRect& rect, const BRect& updateRect,
	float radius, const rgb_color& base, const rgb_color& background, uint32 flags,
	uint32 borders)
{
	_DrawButtonFrame(view, rect, updateRect,  0.0f, 0.0f, 0.0f, 0.0f,
		base, background, 1.0, 1.0, flags, borders);
}


void
HaikuControlLook::DrawButtonFrame(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	const rgb_color& background, uint32 flags,
	uint32 borders)
{
	_DrawButtonFrame(view, rect, updateRect,  0.0f, 0.0f, 0.0f, 0.0f, base, background,
		1.0, 1.0, flags, borders);
}


void
FlatControlLook::DrawMenuFieldBackground(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, bool popupIndicator,
	uint32 flags)
{
	_DrawMenuFieldBackgroundOutside(view, rect, updateRect,
		0.0f, 0.0f, 0.0f, 0.0f, base, popupIndicator, flags);
}


void
FlatControlLook::DrawMenuFieldBackground(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	uint32 borders)
{
	_DrawMenuFieldBackgroundInside(view, rect, updateRect,
		0.0f, 0.0f, 0.0f, 0.0f, base, flags, borders);
}


void
FlatControlLook::DrawMenuFieldBackground(BView* view, BRect& rect,
	const BRect& updateRect, float radius, const rgb_color& base,
	bool popupIndicator, uint32 flags)
{
	_DrawMenuFieldBackgroundOutside(view, rect, updateRect, radius, radius,
		radius, radius, base, popupIndicator, flags);
}


void
FlatControlLook::DrawMenuFieldBackground(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	bool popupIndicator, uint32 flags)
{
	_DrawMenuFieldBackgroundOutside(view, rect, updateRect, leftTopRadius,
		rightTopRadius, leftBottomRadius, rightBottomRadius, base,
		popupIndicator, flags);
}



void
FlatControlLook::_DrawMenuFieldBackgroundOutside(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	bool popupIndicator, uint32 flags)
{
	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	if (popupIndicator) {
		BRect leftRect(rect);
		leftRect.right -= 10;

		BRect rightRect(rect);
		rightRect.left = rightRect.right - 9;

		_DrawMenuFieldBackgroundInside(view, leftRect, updateRect,
			leftTopRadius, 0.0f, leftBottomRadius, 0.0f, base, flags,
			B_LEFT_BORDER | B_TOP_BORDER | B_BOTTOM_BORDER);

		_DrawMenuFieldBackgroundInside(view, rightRect, updateRect,
			0.0f, rightTopRadius, 0.0f, rightBottomRadius, base, flags,
			B_TOP_BORDER | B_RIGHT_BORDER | B_BOTTOM_BORDER);

		_DrawPopUpMarker(view, rightRect, base, flags);

		// draw a line on the left of the popup frame
		rgb_color bevelShadowColor = _BevelShadowColor(base, flags);
		view->SetHighColor(bevelShadowColor);
		BPoint leftTopCorner(floorf(rightRect.left - 1.0),
			floorf(rightRect.top - 1.0));
		BPoint leftBottomCorner(floorf(rightRect.left - 1.0),
			floorf(rightRect.bottom + 1.0));
		view->StrokeLine(leftTopCorner, leftBottomCorner);

		rect = leftRect;
	} else {
		_DrawMenuFieldBackgroundInside(view, rect, updateRect, leftTopRadius,
			rightTopRadius, leftBottomRadius, rightBottomRadius, base, flags,
			B_TOP_BORDER | B_RIGHT_BORDER | B_BOTTOM_BORDER);
	}
}


void
FlatControlLook::_DrawMenuFieldBackgroundInside(BView* view, BRect& rect,
	const BRect& updateRect, float leftTopRadius, float rightTopRadius,
	float leftBottomRadius, float rightBottomRadius, const rgb_color& base,
	uint32 flags, uint32 borders)
{
	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	// save the clipping constraints of the view
	view->PushState();

	// set clipping constraints to updateRect
	BRegion clipping(updateRect);
	view->ConstrainClippingRegion(&clipping);

	// frame colors
	rgb_color frameLightColor  = _FrameLightColor(base, flags);
	rgb_color frameShadowColor = _FrameShadowColor(base, flags);

	// indicator background color
	rgb_color indicatorBase;
	if ((borders & B_LEFT_BORDER) != 0)
		indicatorBase = base;
	else {
		if ((flags & B_DISABLED) != 0)
			indicatorBase = tint_color(base, 1.5);
		else
			indicatorBase = tint_color(base, 0.97);
	}

	// bevel colors
	rgb_color cornerColor = tint_color(indicatorBase, 1.0);
	rgb_color bevelColor1 = tint_color(indicatorBase, 1.0);
	rgb_color bevelColor2 = tint_color(indicatorBase, 1.0);
	rgb_color bevelColor3 = tint_color(indicatorBase, 1.0);

	if ((flags & B_DISABLED) != 0) {
		cornerColor = tint_color(indicatorBase, 1.0);
		bevelColor1 = tint_color(indicatorBase, 1.0);
		bevelColor2 = tint_color(indicatorBase, 1.0);
		bevelColor3 = tint_color(indicatorBase, 1.0);
	} else {
		cornerColor = tint_color(indicatorBase, 1.0);
		bevelColor1 = tint_color(indicatorBase, 1.0);
		bevelColor2 = tint_color(indicatorBase, 1.0);
		bevelColor3 = tint_color(indicatorBase, 1.0);
	}

	// surface top gradient
	BGradientLinear fillGradient;
	_MakeButtonGradient(fillGradient, rect, indicatorBase, flags);

	// rounded corners

	if ((borders & B_LEFT_BORDER) != 0 && (borders & B_TOP_BORDER) != 0
		&& leftTopRadius > 0) {
		// draw left top rounded corner
		BRect leftTopCorner(floorf(rect.left), floorf(rect.top),
			floorf(rect.left + leftTopRadius - 2.0),
			floorf(rect.top + leftTopRadius - 2.0));
		clipping.Exclude(leftTopCorner);

		BRegion cornerClipping(leftTopCorner);
		view->ConstrainClippingRegion(&cornerClipping);

		BRect ellipseRect(leftTopCorner);
		ellipseRect.InsetBy(-1.0, -1.0);
		ellipseRect.right = ellipseRect.left + ellipseRect.Width() * 2;
		ellipseRect.bottom = ellipseRect.top + ellipseRect.Height() * 2;

		// draw the frame (again)
		view->SetHighColor(frameLightColor);
		view->FillEllipse(ellipseRect);

		// draw the bevel and background
		_DrawRoundCornerBackgroundLeftTop(view, leftTopCorner, updateRect,
			bevelColor1, fillGradient);
	}

	if ((borders & B_TOP_BORDER) != 0 && (borders & B_RIGHT_BORDER) != 0
		&& rightTopRadius > 0) {
		// draw right top rounded corner
		BRect rightTopCorner(floorf(rect.right - rightTopRadius + 2.0),
			floorf(rect.top), floorf(rect.right),
			floorf(rect.top + rightTopRadius - 2.0));
		clipping.Exclude(rightTopCorner);

		BRegion cornerClipping(rightTopCorner);
		view->ConstrainClippingRegion(&cornerClipping);

		BRect ellipseRect(rightTopCorner);
		ellipseRect.InsetBy(-1.0, -1.0);
		ellipseRect.left = ellipseRect.right - ellipseRect.Width() * 2;
		ellipseRect.bottom = ellipseRect.top + ellipseRect.Height() * 2;

		// draw the frame (again)
		if (frameLightColor == frameShadowColor) {
			view->SetHighColor(frameLightColor);
			view->FillEllipse(ellipseRect);
		} else {
			BGradientLinear gradient;
			gradient.AddColor(frameLightColor, 0);
			gradient.AddColor(frameShadowColor, 255);
			gradient.SetStart(rightTopCorner.LeftTop());
			gradient.SetEnd(rightTopCorner.RightBottom());
			view->FillEllipse(ellipseRect, gradient);
		}

		// draw the bevel and background
		_DrawRoundCornerBackgroundRightTop(view, rightTopCorner, updateRect,
			bevelColor1, bevelColor3, fillGradient);
	}

	if ((borders & B_LEFT_BORDER) != 0 && (borders & B_BOTTOM_BORDER) != 0
		&& leftBottomRadius > 0) {
		// draw left bottom rounded corner
		BRect leftBottomCorner(floorf(rect.left),
			floorf(rect.bottom - leftBottomRadius + 2.0),
			floorf(rect.left + leftBottomRadius - 2.0),
			floorf(rect.bottom));
		clipping.Exclude(leftBottomCorner);

		BRegion cornerClipping(leftBottomCorner);
		view->ConstrainClippingRegion(&cornerClipping);

		BRect ellipseRect(leftBottomCorner);
		ellipseRect.InsetBy(-1.0, -1.0);
		ellipseRect.right = ellipseRect.left + ellipseRect.Width() * 2;
		ellipseRect.top = ellipseRect.bottom - ellipseRect.Height() * 2;

		// draw the frame (again)
		if (frameLightColor == frameShadowColor) {
			view->SetHighColor(frameLightColor);
			view->FillEllipse(ellipseRect);
		} else {
			BGradientLinear gradient;
			gradient.AddColor(frameLightColor, 0);
			gradient.AddColor(frameShadowColor, 255);
			gradient.SetStart(leftBottomCorner.LeftTop());
			gradient.SetEnd(leftBottomCorner.RightBottom());
			view->FillEllipse(ellipseRect, gradient);
		}

		// draw the bevel and background
		_DrawRoundCornerBackgroundLeftBottom(view, leftBottomCorner,
			updateRect, bevelColor2, bevelColor3, fillGradient);
	}

	if ((borders & B_RIGHT_BORDER) != 0 && (borders & B_BOTTOM_BORDER) != 0
		&& rightBottomRadius > 0) {
		// draw right bottom rounded corner
		BRect rightBottomCorner(floorf(rect.right - rightBottomRadius + 2.0),
			floorf(rect.bottom - rightBottomRadius + 2.0), floorf(rect.right),
			floorf(rect.bottom));
		clipping.Exclude(rightBottomCorner);

		BRegion cornerClipping(rightBottomCorner);
		view->ConstrainClippingRegion(&cornerClipping);

		BRect ellipseRect(rightBottomCorner);
		ellipseRect.InsetBy(-1.0, -1.0);
		ellipseRect.left = ellipseRect.right - ellipseRect.Width() * 2;
		ellipseRect.top = ellipseRect.bottom - ellipseRect.Height() * 2;

		// draw the frame (again)
		view->SetHighColor(frameShadowColor);
		view->FillEllipse(ellipseRect);

		// draw the bevel and background
		_DrawRoundCornerBackgroundRightBottom(view, rightBottomCorner,
			updateRect, bevelColor3, fillGradient);
	}

	// clip out the corners
	view->ConstrainClippingRegion(&clipping);

	// draw the bevel
	_DrawFrame(view, rect,
		bevelColor2, bevelColor1,
		bevelColor3, bevelColor3,
		cornerColor, cornerColor,
		borders);

	// fill in the background
	view->FillRect(rect, fillGradient);

	// restore the clipping constraints of the view
	view->PopState();
}


void
FlatControlLook::DrawRaisedBorder(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	uint32 borders)
{
	rgb_color lightColor;
	rgb_color shadowColor;

	if ((flags & B_DISABLED) != 0) {
		lightColor = base;
		shadowColor = base;
	} else {
		lightColor = tint_color(base, 1.0);
		shadowColor = tint_color(base, 1.0);
	}

	_DrawFrame(view, rect, shadowColor, shadowColor, shadowColor, shadowColor,
		borders);
}

void
FlatControlLook::_DrawOuterResessedFrame(BView* view, BRect& rect,
	const rgb_color& base, float contrast, float brightness, uint32 flags,
	uint32 borders)
{
	rgb_color edgeLightColor = _EdgeLightColor(base, contrast,
		brightness, flags);
	rgb_color edgeShadowColor = _EdgeShadowColor(base, contrast,
		brightness, flags);

	if ((flags & B_BLEND_FRAME) != 0) {
		// assumes the background has already been painted
		drawing_mode oldDrawingMode = view->DrawingMode();
		view->SetDrawingMode(B_OP_ALPHA);

		_DrawFrame(view, rect, edgeShadowColor, edgeShadowColor,
			edgeLightColor, edgeLightColor, borders);

		view->SetDrawingMode(oldDrawingMode);
	} else {
		_DrawFrame(view, rect, edgeShadowColor, edgeShadowColor,
			edgeShadowColor, edgeShadowColor, borders);
	}
}

rgb_color
FlatControlLook::_EdgeLightColor(const rgb_color& base, float contrast,
	float brightness, uint32 flags)
{
	rgb_color edgeLightColor;

	if ((flags & B_BLEND_FRAME) != 0) {
		uint8 alpha = uint8(20 * contrast);
		uint8 white = uint8(255 * brightness);

		edgeLightColor = (rgb_color){ white, white, white, alpha };
	} else {
		// colors
		float tintLight = kEdgeBevelLightTint;

		if (contrast == 0.0)
			tintLight = B_NO_TINT;
		else if (contrast != 1.0)
			tintLight = B_NO_TINT + (tintLight - B_NO_TINT) * contrast;

		edgeLightColor = tint_color(base, tintLight);

		if (brightness < 1.0) {
			edgeLightColor.red = uint8(edgeLightColor.red * brightness);
			edgeLightColor.green = uint8(edgeLightColor.green * brightness);
			edgeLightColor.blue = uint8(edgeLightColor.blue * brightness);
		}
	}

	return edgeLightColor;
}


rgb_color
FlatControlLook::_EdgeShadowColor(const rgb_color& base, float contrast,
	float brightness, uint32 flags)
{
	rgb_color edgeShadowColor;

	if ((flags & B_BLEND_FRAME) != 0) {
		uint8 alpha = uint8(10 * contrast);
		edgeShadowColor = (rgb_color){ 0, 0, 0, alpha };
	} else {
		float tintShadow = kEdgeBevelShadowTint;

		if (contrast == 0.0)
			tintShadow = B_NO_TINT;
		else if (contrast != 1.0)
			tintShadow = B_NO_TINT + (tintShadow - B_NO_TINT) * contrast;

		edgeShadowColor = tint_color(base, tintShadow);

		if (brightness < 1.0) {
			edgeShadowColor.red = uint8(edgeShadowColor.red * brightness);
			edgeShadowColor.green = uint8(edgeShadowColor.green * brightness);
			edgeShadowColor.blue = uint8(edgeShadowColor.blue * brightness);
		}
	}

	return edgeShadowColor;
}

void
FlatControlLook::DrawBorder(BView* view, BRect& rect, const BRect& updateRect,
	const rgb_color& base, border_style borderStyle, uint32 flags,
	uint32 borders)
{
	if (borderStyle == B_NO_BORDER)
		return;
	//rgb_color scrollbarFrameColor = tint_color(base, B_DARKEN_1_TINT);

	rgb_color scrollbarFrameColor = tint_color(base, 1.025);
	if (base.red + base.green + base.blue <= 128 * 3) {
		scrollbarFrameColor = tint_color(base, 1.025);
	}

	if ((flags & B_FOCUSED) != 0)
		scrollbarFrameColor = tint_color(base, 1.025);

	if (borderStyle == B_FANCY_BORDER)
		_DrawOuterResessedFrame(view, rect, base, 1.0, 1.0, flags, borders);

	_DrawFrame(view, rect, scrollbarFrameColor, scrollbarFrameColor,
		scrollbarFrameColor, scrollbarFrameColor, borders);
}

void
HaikuControlLook::DrawTextControlBorder(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, uint32 flags,
	uint32 borders)
{
	if (!rect.Intersects(updateRect))
		return;

	rgb_color dark1BorderColor;
	rgb_color dark2BorderColor;
	rgb_color navigationColor = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
	rgb_color invalidColor = ui_color(B_FAILURE_COLOR);

	if ((flags & B_DISABLED) != 0) {
		_DrawOuterResessedFrame(view, rect, base, 0.0, 1.0, flags, borders);

		if ((flags & B_BLEND_FRAME) != 0)
			dark1BorderColor = (rgb_color){ 0, 0, 0, 40 };
		else
			dark1BorderColor = tint_color(base, 1.0);
		dark2BorderColor = dark1BorderColor;
	} else if ((flags & B_CLICKED) != 0) {
		dark1BorderColor = tint_color(base, 1.0);
		dark2BorderColor = tint_color(base, 1.0);

		// BCheckBox uses this to indicate the clicked state...
		_DrawFrame(view, rect,
			dark1BorderColor, dark1BorderColor,
			dark2BorderColor, dark2BorderColor);

		dark2BorderColor = dark1BorderColor;
	} else {
		_DrawOuterResessedFrame(view, rect, base, 0.6, 1.0, flags, borders);

		if ((flags & B_BLEND_FRAME) != 0) {
			dark1BorderColor = (rgb_color){ 0, 0, 0, 102 };
			dark2BorderColor = (rgb_color){ 0, 0, 0, 97 };
		} else {
			dark1BorderColor = tint_color(base, 1.10);
			dark2BorderColor = tint_color(base, 1.0);
		}
	}

	if ((flags & B_DISABLED) == 0 && (flags & B_FOCUSED) != 0) {
		dark1BorderColor = navigationColor;
		dark2BorderColor = navigationColor;
	}

	if ((flags & B_DISABLED) == 0 && (flags & B_INVALID) != 0) {
		dark1BorderColor = invalidColor;
		dark2BorderColor = invalidColor;
	}

	if ((flags & B_BLEND_FRAME) != 0) {
		drawing_mode oldMode = view->DrawingMode();
		view->SetDrawingMode(B_OP_ALPHA);

		_DrawFrame(view, rect,
			dark1BorderColor, dark1BorderColor,
			dark2BorderColor, dark2BorderColor, borders);

		view->SetDrawingMode(oldMode);
	} else {
		_DrawFrame(view, rect,
			dark1BorderColor, dark1BorderColor,
			dark2BorderColor, dark2BorderColor, borders);
	}
}

void
HaikuControlLook::_DrawOuterResessedFrame(BView* view, BRect& rect,
	const rgb_color& base, float contrast, float brightness, uint32 flags,
	uint32 borders)
{
	rgb_color edgeLightColor = _EdgeLightColor(base, contrast,
		brightness, flags);
	rgb_color edgeShadowColor = _EdgeShadowColor(base, contrast,
		brightness, flags);

	if ((flags & B_BLEND_FRAME) != 0) {
		// assumes the background has already been painted
		drawing_mode oldDrawingMode = view->DrawingMode();
		view->SetDrawingMode(B_OP_ALPHA);

		_DrawFrame(view, rect, edgeShadowColor, edgeShadowColor,
			edgeShadowColor, edgeShadowColor, borders);

		view->SetDrawingMode(oldDrawingMode);
	} else {
		_DrawFrame(view, rect, edgeShadowColor, edgeShadowColor,
			edgeShadowColor, edgeShadowColor, borders);
	}
}

void
HaikuControlLook::DrawScrollViewFrame(BView* view, BRect& rect,
	const BRect& updateRect, BRect verticalScrollBarFrame,
	BRect horizontalScrollBarFrame, const rgb_color& base,
	border_style borderStyle, uint32 flags, uint32 _borders)
{
	// calculate scroll corner rect before messing with the "rect"
	BRect scrollCornerFillRect(rect.right, rect.bottom,
		rect.right, rect.bottom);

	if (horizontalScrollBarFrame.IsValid())
		scrollCornerFillRect.left = horizontalScrollBarFrame.right + 1;

	if (verticalScrollBarFrame.IsValid())
		scrollCornerFillRect.top = verticalScrollBarFrame.bottom + 1;

	if (borderStyle == B_NO_BORDER) {
		if (scrollCornerFillRect.IsValid()) {
			view->SetHighColor(base);
			view->FillRect(scrollCornerFillRect);
		}
		return;
	}

	bool excludeScrollCorner = borderStyle == B_FANCY_BORDER
		&& horizontalScrollBarFrame.IsValid()
		&& verticalScrollBarFrame.IsValid();

	uint32 borders = _borders;
	if (excludeScrollCorner) {
		rect.bottom = horizontalScrollBarFrame.top;
		rect.right = verticalScrollBarFrame.left;
		borders &= ~(B_RIGHT_BORDER | B_BOTTOM_BORDER);
	}

	rgb_color scrollbarFrameColor = tint_color(base, B_DARKEN_2_TINT);

	if (borderStyle == B_FANCY_BORDER)
		_DrawOuterResessedFrame(view, rect, base, 1.0, 1.0, flags, borders);

	if ((flags & B_FOCUSED) != 0) {
		rgb_color focusColor = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
		_DrawFrame(view, rect, focusColor, focusColor, focusColor, focusColor,
			borders);
	} else {
		_DrawFrame(view, rect, scrollbarFrameColor, scrollbarFrameColor,
			scrollbarFrameColor, scrollbarFrameColor, borders);
	}

	if (excludeScrollCorner) {
		horizontalScrollBarFrame.InsetBy(-1, -1);
		// do not overdraw the top edge
		horizontalScrollBarFrame.top += 2;
		borders = _borders;
		borders &= ~B_TOP_BORDER;
		_DrawOuterResessedFrame(view, horizontalScrollBarFrame, base,
			1.0, 1.0, flags, borders);
		_DrawFrame(view, horizontalScrollBarFrame, scrollbarFrameColor,
			scrollbarFrameColor, scrollbarFrameColor, scrollbarFrameColor,
			borders);

		verticalScrollBarFrame.InsetBy(-1, -1);
		// do not overdraw the left edge
		verticalScrollBarFrame.left += 2;
		borders = _borders;
		borders &= ~B_LEFT_BORDER;
		_DrawOuterResessedFrame(view, verticalScrollBarFrame, base,
			1.0, 1.0, flags, borders);
		_DrawFrame(view, verticalScrollBarFrame, scrollbarFrameColor,
			scrollbarFrameColor, scrollbarFrameColor, scrollbarFrameColor,
			borders);

		// exclude recessed frame
		scrollCornerFillRect.top++;
		scrollCornerFillRect.left++;
	}

	if (scrollCornerFillRect.IsValid()) {
		view->SetHighColor(base);
		view->FillRect(scrollCornerFillRect);
	}
}

rgb_color
HaikuControlLook::_BevelLightColor(const rgb_color& base, uint32 flags)
{
	rgb_color bevelLightColor = tint_color(base, 1.0);

	if ((flags & B_DISABLED) != 0)
		bevelLightColor = tint_color(base, B_DARKEN_1_TINT);

	if ((flags & B_ACTIVATED) != 0)
		bevelLightColor = tint_color(base, B_DARKEN_1_TINT);

	return bevelLightColor;
}


rgb_color
HaikuControlLook::_BevelShadowColor(const rgb_color& base, uint32 flags)
{
	rgb_color bevelShadowColor = tint_color(base, 1.0);

	if ((flags & B_DISABLED) != 0)
		bevelShadowColor = base;

	if ((flags & B_ACTIVATED) != 0)
		bevelShadowColor = tint_color(base, B_DARKEN_1_TINT);

	return bevelShadowColor;
}

extern "C" BControlLook* (instantiate_control_look)(image_id id)
{
	return new (std::nothrow)FlatControlLook(id);
}
