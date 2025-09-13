#pragma once
#include "Mesh.h"
#include "UIVertex.h"
#include "RenderObject.h"

namespace Quasi::Graphics {
    struct DrawAttributes {
        Math::fColor fillColor = 1;
        Math::fColor strokeColor = 0;
        float strokeWeight = 5;
        u32 drawStyle = 0;
    };

    struct Gradient {
        Math::fv2    startPoint, direction;
        Math::fColor startColor, endColor;

        static Gradient Vertical(float s, float e, const Math::fColor& sc, const Math::fColor& ec);
        static Gradient Horizontal(float s, float e, const Math::fColor& sc, const Math::fColor& ec);

        Math::fColor At(const Math::fv2& p) const;
    };

    class UIMesh : public Mesh<UIVertex> {
    public:
        void SetTextureFill();
        void FillGradient(const Gradient& g);
        void OverlayGradient(const Gradient& g);
    };

    class Canvas {
        RenderObject<UIVertex> renderCanvas;
        UIMesh worldMesh;
        OptRef<UIMesh> drawMesh = nullptr; // can be set to any mesh. by default it draws to the world mesh
        DrawAttributes drawAttr;
    public:
        Math::Transform2D transform;


        Canvas();
        Canvas(GraphicsDevice& gd);

        enum ArcMode {
            OPEN, CHORD, CLOSED
        };
        enum ArcDirection {
            CCW, CW, MAJOR, MINOR
        };

        void DrawTriangle(const Math::fv2& p1, const Math::fv2& p2, const Math::fv2& p3);
        void DrawQuad(const Math::fv2& p1, const Math::fv2& p2, const Math::fv2& p3, const Math::fv2& p4);
        void DrawSquare(const Math::fv2& center, float size);
        void DrawRect(const Math::fRect2D& rect);
        void DrawRoundedSquare(const Math::fv2& center, float size, float radius);
        void DrawVarRoundedSquare(const Math::fv2& center, float size, float tr, float br, float tl, float bl);
        void DrawRoundedRect(const Math::fRect2D& rect, float radius);
        void DrawVarRoundedRect(const Math::fRect2D& rect, float tr, float br, float tl, float bl);
        void DrawCircle(const Math::fv2& center, float radius);
        void DrawEllipse(const Math::fRect2D& bounds);
        void DrawArc(const Math::fv2& center, float radius, const Math::Rotor2D& startAngle, const Math::Rotor2D& endAngle, ArcDirection direction = CCW, ArcMode mode = OPEN);
        void DrawArcCCW(const Math::fv2& center, float radius, const Math::Rotor2D& startAngle, const Math::Rotor2D& endAngle, ArcMode mode = OPEN);
        // void DrawEllArc(const Math::fv2& center, const Math::fv2& size, float startAngle, float stopAngle, ArcMode mode = OPEN);
        void DrawPoint(const Math::fv2& position);
        void DrawLine(const Math::fv2& start, const Math::fv2& end);

        void DrawMesh(const UIMesh& mesh);

        UIMesh& DestinationMesh();
        struct Batch {
            Canvas& canvas;
            UIMesh& mesh;
            u32 iOffset;
            UIVertex storedPoint;

            void PushV(UIVertex v);
            void ResizeV(u32) const {}
            void ReserveV(u32) const {}
            UIVertex& VertAt(u32 i);
            const UIVertex& VertAt(u32 i) const;
            u32 VertCount() const;

            void ResizeI(u32) const {}
            void ReserveI(u32) const {}
            void PushI(u32 i, u32 j, u32 k);
            TriIndices* IndexData();

            void SetColor(const Math::fColor& color);
            void SetFill();
            void SetStroke();
            void SetPrim(u32 prim);
            void SetPrimAndStyle(u32 primAndStyle);
            void SetPosition(const Math::fv2& point);
            void SetUV(float u, float v);
            void SetSTUV(float s, float t, float u, float v = 0);
            void Point(const Math::fv2& position);
            void PointCirc(const Math::fv2& position, float u, float v, float radiusRatio = 0);
            void PointQBezGeneric(const Math::fv2& position, float u, float v, bool flip); // start
            void PointQBezA(const Math::fv2& position, bool flip); // start
            void PointQBezB(const Math::fv2& position, bool flip); // control
            void PointQBezC(const Math::fv2& position, bool flip); // end
            void PointQTriple(const Math::fv2& position, bool innerSide); // flat + start + end
            void PointQDoubleS(const Math::fv2& position, bool innerSide); // flat + start
            void PointQDoubleE(const Math::fv2& position, bool innerSide); // flat + end
            void PointQDoubleC(const Math::fv2& position, bool innerSide); // flat + control
            void PointsQBez(Span<const Math::fv2> bezpoints, bool innerSide);
            void PointsQBezClosed(Span<const Math::fv2> outer, Span<const Math::fv2> inner);
            void Tri(u32 i, u32 j, u32 k);
            void Quad(u32 i, u32 j, u32 k, u32 l);
            void TriStrip(Span<const u32> strip);
            void TriFan(Span<const u32> fan);
            void Push();
            void PushAsPlain();

            void Refresh();
        };
        Batch NewBatch();

    private:
        bool NeedDrawFill()   const { return !(drawAttr.drawStyle & UIRender::NO_FILL); }
        bool NeedDrawStroke() const { return !(drawAttr.drawStyle & UIRender::NO_STROKE); }

        void DrawSimpleLine(const Math::fv2& start, const Math::fv2& end, const Math::fv2& tangent);
        void DrawLineCap(const Math::fv2& point, const Math::fv2& tangent);
        void DrawLineJoin(const Math::fv2& point, const Math::fv2& tangent1, const Math::fv2& tangent2);

        // draws a semicircle (pi) -> none (0)
        // forward is the middle of the section (scaled with radius)
        // cos is half of the angle of the full section, so is sin
        void DrawHalfCircularChord(const Math::fv2& center, const Math::fv2& forward, float cos, float sin, const Math::fColor& color);
        void DrawHalfCircle(const Math::fv2& center, const Math::fv2& forward, const Math::fColor& color);
        void DrawSimpleCircle(const Math::fv2& center, float radius, const Math::fColor& color);
        void DrawQuarterChord(const Math::fv2& center, const Math::fv2& rightMostSide, const Math::fColor& color);
        void DrawSimpleEllipse(const Math::fv2& center, const Math::fv2& radii, const Math::fColor& color);
        void DrawSimpleEllipse(const Math::fRect2D& bounds, const Math::fColor& color);
        void DrawCircleOutline(const Math::fv2& center, float radius, float thickness, const Math::fColor& color);

        // by default the arc is on the first quadrant, which can be changed by start
        void DrawQuarterArc(const Math::fv2& center, const Math::Rotor2D& start, float radius, float thickness, const Math::fColor& color);
        void DrawCircularArcCCW(const Math::fv2& center, const Math::Rotor2D& mid, Math::Rotor2D step, float radius, float thickness, const Math::fColor& color);

        void DrawSimpleRect(const Math::fRect2D& rect, const Math::fColor& color);
        void DrawSimpleRoundedRect(const Math::fRect2D& outer, float radius, const Math::fColor& color);
        void DrawSimpleVarRoundRect(const Math::fRect2D& outer, float tr, float br, float tl, float bl, const Math::fColor& color);
        void DrawRectStroke(const Math::fRect2D& rect);
    public:

        enum CurveMode {
            OPEN_CURVE, CLOSED_CURVE
        };
        struct Path {
            Canvas& canvas;
            u32 closing = OPEN_CURVE;
            enum {
                OPEN_CURVE_SECOND_POINT = 2,
                OPEN_CURVE_MIDDLE_POINT = 3,
                CLOSED_CURVE_SECOND_POINT = 4,
                CLOSED_CURVE_MIDDLE_POINT = 5,
                COMPLETED = -1,
            };
        private:
            Math::fv2 firstPoint, firstTangent, lastPoint, lastTangent;

            bool NoPointsYet();
            void BeginLineSegment(const Math::fv2& p);
            void EndLineSegment(const Math::fv2& p, const Math::fv2& tangent);

            void DrawCircularArcCCW(const Math::fv2& startPoint, const Math::fv2& center, const Math::Rotor2D& turn);
        public:
            Path(Canvas& canvas, CurveMode mode) : canvas(canvas), closing(mode) {}
            void AddPoint(const Math::fv2& point);
            // from lastPoint to current point is a full turn
            void AddCircularArc(const Math::fv2& center, const Math::Rotor2D& turn, ArcDirection dir = CCW);
            void AddQuadBez(const Math::fv2& control, const Math::fv2& end);
            void ClosePath();
            ~Path();
        };
        Path NewPath(CurveMode mode = CLOSED_CURVE);

        DrawAttributes& DrawAttr() { return drawAttr; }
        const DrawAttributes& DrawAttr() const { return drawAttr; }

        void StrokeWeight(float weight);
        void StrokeCap(UIRender::RenderStyle cap);
        void StrokeJoin(UIRender::RenderStyle join);

        void Fill(const Math::fColor& fillColor);
        void Stroke(const Math::fColor& strokeColor);
        void NoStroke();
        void NoFill();

        struct DeferRenderScope {
            Canvas& canvas;
            OptRef<UIMesh> prevDestination;
            DeferRenderScope(Canvas& canvas, UIMesh& newDest);
            ~DeferRenderScope();
        };
        DeferRenderScope RenderTo(UIMesh& mesh);

        struct PushStylesScope {
            Canvas& canvas;
            DrawAttributes originalAttr;
            PushStylesScope(Canvas& canvas);
            ~PushStylesScope();
        };
        PushStylesScope PushStyles();

        Math::fv2 TransformToWorldSpace(const Math::fv2& point) const;

        void BeginFrame();
        void EndFrame();
    };
}
