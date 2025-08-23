#include "Canvas.h"
#include "GraphicsDevice.h"

namespace Quasi::Graphics {
    Canvas::Canvas() {}
    Canvas::Canvas(GraphicsDevice& gd) : renderCanvas(gd.CreateNewRender<UIVertex>(16384, 16384)) {
        const Math::fv2 screenSize = gd.GetWindowSize().As<float>();
        renderCanvas.SetProjection(Math::Matrix3D::OrthoProjection({ 0, screenSize.AddZ(1) }));

        renderCanvas.UseShader(
            "#shader vertex\n"
            "#version 330 core\n"
            "layout (location = 0) in vec2 position;"
            "layout (location = 1) in vec2 texCoord;"
            "layout (location = 2) in vec4 color;"
            "layout (location = 3) in vec4 stuv;"
            "layout (location = 4) in int renderPrim;"
            ""
            "out vec4 vColor;"
            "out vec4 vSTUV;"
            "flat out int vRenderPrim;"
            ""
            "uniform mat4 u_projection, u_view;"
            ""
            "void main() {"
            "    gl_Position = u_projection * u_view * vec4(position, 0.0, 1.0);"
            "    vColor = color;"
            "    vSTUV = stuv;"
            "    vRenderPrim = renderPrim;"
            "}\n"
            "#shader fragment\n"
            "#version 330 core\n"
            ""
            "layout (location = 0) out vec4 FragColor;"
            ""
            "in vec4 vColor;"
            "in vec4 vSTUV;"
            "flat in int vRenderPrim;"
            ""
            "void main() {"
            "    switch (vRenderPrim) {"
            "        case 0: FragColor = vColor; break;"
            "        case 1: {"
            "            float distSq = vSTUV.x * vSTUV.x + vSTUV.y * vSTUV.y;"
            "            if (distSq <= 1.0 && vSTUV.z <= distSq) {"
            "                FragColor = vColor;"
            "            } else "
            // "                FragColor = vec4(1 - vColor.xyz, 1);"
            "                discard;"
            "            break;"
            "        }"
            "    }"
            "}"
        );
    }

    void Canvas::DrawTriangle(const Math::fv2& p1, const Math::fv2& p2, const Math::fv2& p3) {
        if (NeedDrawFill()) {
            Batch batch = NewBatch();
            batch.PushV(UIVertex { p1, 0, drawAttr.fillColor, 0 });
            batch.PushV(UIVertex { p2, 0, drawAttr.fillColor, 0 });
            batch.PushV(UIVertex { p3, 0, drawAttr.fillColor, 0 });
            batch.PushI(0, 1, 2);
        }
        if (NeedDrawStroke()) QWith$(p = Path(*this, CLOSED_CURVE)) {
            p.AddPoint(p1);
            p.AddPoint(p2);
            p.AddPoint(p3);
        }
    }

    void Canvas::DrawQuad(const Math::fv2& p1, const Math::fv2& p2, const Math::fv2& p3, const Math::fv2& p4) {
        if (NeedDrawFill()) {
            Batch batch = NewBatch();
            batch.PushV(UIVertex { p1, 0, drawAttr.fillColor, 0 });
            batch.PushV(UIVertex { p2, 0, drawAttr.fillColor, 0 });
            batch.PushV(UIVertex { p3, 0, drawAttr.fillColor, 0 });
            batch.PushV(UIVertex { p4, 0, drawAttr.fillColor, 0 });
            batch.PushI(0, 1, 2);
            batch.PushI(2, 1, 3);
        }
        if (NeedDrawStroke()) QWith$(p = Path(*this, CLOSED_CURVE)) {
            p.AddPoint(p1);
            p.AddPoint(p2);
            p.AddPoint(p3);
            p.AddPoint(p4);
        }
    }

    void Canvas::DrawSquare(const Math::fv2& center, float size) {
        DrawRect(Math::fRect2D::FromCenter(center, size));
    }

    void Canvas::DrawRect(const Math::fRect2D& rect) {
        if (NeedDrawFill()) {
            DrawSimpleRect(rect, drawAttr.fillColor);
        }
        if (NeedDrawStroke()) {
            DrawRectStroke(rect);
        }
    }

    void Canvas::DrawCircle(const Math::fv2& center, float radius) {
        if (NeedDrawFill())
            DrawSimpleCircle(center, radius, drawAttr.fillColor);
        if (NeedDrawStroke())
            DrawCircleOutline(center, radius, drawAttr.strokeWeight, drawAttr.strokeColor);
    }

    void Canvas::DrawEllipse(const Math::fRect2D& bounds) {
        // TODO: this is not a proper offset ellipse;
        //       the stroke of an ellipse is an order-8 algaebraic curve
        //       this is a temporary solution
        //       use cubic beziers next time
        if (NeedDrawFill())
            DrawSimpleEllipse(bounds, drawAttr.fillColor);
        // if (NeedDrawStroke()) {
        //     const Math::fRect2D rect = bounds.Extrude(drawAttr.strokeWeight);
        //     Batch batch = NewBatch();
        //     static constexpr float R2 = Math::ROOT_2;
        //     const float diagonal = R2 * (radius + thickness);
        //     float inner = (radius - thickness) / (radius + thickness);
        //     inner *= inner;
        //     batch.PushV(UIVertex { bounds.min,                     0, drawAttr.strokeColor, { -1, -1, inner, 0 }, UIRender::CIRCLE });
        //     batch.PushV(UIVertex { { bounds.max.x, bounds.min.y }, 0, drawAttr.strokeColor, { +1, -1, inner, 0 }, UIRender::CIRCLE });
        //     batch.PushV(UIVertex { { bounds.min.x, bounds.max.y }, 0, drawAttr.strokeColor, { -1, +1, inner, 0 }, UIRender::CIRCLE });
        //     batch.PushV(UIVertex { bounds.max,                     0, drawAttr.strokeColor, { +1, +1, inner, 0 }, UIRender::CIRCLE });
        //     batch.PushI(0, 1, 2);
        //     batch.PushI(2, 0, 3);
        // }
    }

    void Canvas::DrawArc(const Math::fv2& center, float radius, const Math::Rotor2D& startAngle, const Math::Rotor2D& endAngle, ArcDirection direction, ArcMode mode) {
        switch (direction) {
            case CCW:
                return DrawArcCCW(center, radius, startAngle, endAngle, mode);
            case CW:
                return DrawArcCCW(center, radius, endAngle, startAngle, mode);
            case MAJOR: {
                const bool alreadyMajor = (endAngle - startAngle).Sin() < 0;
                return DrawArcCCW(
                    center, radius,
                    alreadyMajor ? startAngle : endAngle,
                    alreadyMajor ? endAngle : startAngle,
                    mode
                );
            }
            case MINOR: {
                const bool alreadyMinor = (endAngle - startAngle).Sin() > 0;
                return DrawArcCCW(
                    center, radius,
                    alreadyMinor ? startAngle : endAngle,
                    alreadyMinor ? endAngle : startAngle,
                    mode
                );
            }
        }
    }

    void Canvas::DrawArcCCW(const Math::fv2& center, float radius, const Math::Rotor2D& startAngle, const Math::Rotor2D& endAngle, ArcMode mode) {
        Math::fComplex turn = (endAngle - startAngle).AsComplex();
        Math::fv2 start = startAngle.IHat() * radius;
        if (NeedDrawFill()) {
            const bool isMajorArc = turn.im < 0;
            if (isMajorArc) { // needs to split into two sections
                DrawHalfCircle(center, start.PerpendLeft(), drawAttr.fillColor);
                turn = -turn;
                start = -start;
            }
            const Math::fComplex halfTurn = Math::Rotor2D::FromComplex(turn).Halved().AsComplex();
            DrawHalfCircularChord(center, start.ComplexMul(halfTurn.AsVec2D()), halfTurn.re, std::abs(halfTurn.im), drawAttr.fillColor);

            if (mode != CHORD || isMajorArc) {
                Batch batch = NewBatch();
                batch.PushV({ center + start, 0, drawAttr.fillColor, 0 });
                batch.PushV({ center + radius * endAngle.IHat(), 0, drawAttr.fillColor, 0 });
                if (mode == CHORD) {
                    batch.PushV({ center - start, 0, drawAttr.fillColor, 0 });
                } else {
                    batch.PushV({ center, 0, drawAttr.fillColor, 0 });
                }
                batch.PushI(0, 1, 2);
            }

            if (isMajorArc) {
                // restore fixes
                turn = -turn;
                start = -start;
            }
        }
        if (NeedDrawStroke()) {
            QWith$(p = Path(*this, mode == OPEN ? OPEN_CURVE : CLOSED_CURVE)) {
                p.AddPoint(center + start);
                p.AddCircularArc(center, Math::Rotor2D::FromComplex(turn), CCW);
                if (mode == CLOSED) {
                    p.AddPoint(center);
                }
            }
        }
    }

    void Canvas::DrawEllArc(const Math::fv2& center, const Math::fv2& size, float startAngle, float stopAngle,
                            ArcMode mode) {
        // TODO
    }

    void Canvas::DrawPoint(const Math::fv2& position) {
        if (NeedDrawStroke())
            DrawSimpleCircle(position, drawAttr.strokeWeight, drawAttr.strokeColor);
    }

    void Canvas::DrawLine(const Math::fv2& start, const Math::fv2& end) {
        const Math::fv2 tangent = start.DirTowards(end).Norm();
        DrawSimpleLine(start, end, tangent);
        DrawLineCap(start, tangent);
        DrawLineCap(end,  -tangent);
    }

    void Canvas::Batch::PushV(UIVertex v) {
        v.Position = canvas.TransformToWorldSpace(v.Position);
        canvas.renderCanvas->PushVertex(v);
    }
    UIVertex& Canvas::Batch::VertAt(u32 i) { return canvas.renderCanvas->vertexData.AsSpanMut().Transmute<UIVertex>()[i]; }
    const UIVertex& Canvas::Batch::VertAt(u32 i) const { return canvas.renderCanvas->vertexData.AsSpan().Transmute<UIVertex>()[i]; }
    u32 Canvas::Batch::VertCount() const { return canvas.renderCanvas->vertexOffset / sizeof(UIVertex) - iOffset; }
    void Canvas::Batch::PushI(u32 i, u32 j, u32 k) { canvas.renderCanvas->PushIndex({ iOffset + i, iOffset + j, iOffset + k }); }
    TriIndices* Canvas::Batch::IndexData() { return canvas.renderCanvas->indexData.AsSpanMut().Transmute<TriIndices>().Data(); }
    void Canvas::Batch::Refresh() { iOffset = canvas.renderCanvas->vertexOffset / sizeof(UIVertex); }

    Canvas::Batch Canvas::NewBatch() {
        return { *this, (u32)(renderCanvas->vertexOffset / sizeof(UIVertex)) };
    }

    void Canvas::DrawSimpleLine(const Math::fv2& start, const Math::fv2& end, const Math::fv2& tangent) {
        Batch batch = NewBatch();
        const Math::fv2 normal = tangent.Perpend() * drawAttr.strokeWeight;
        batch.PushV(UIVertex { start + normal, 0, drawAttr.strokeColor, 0 });
        batch.PushV(UIVertex { start - normal, 0, drawAttr.strokeColor, 0 });
        batch.PushV(UIVertex { end   - normal, 0, drawAttr.strokeColor, 0 });
        batch.PushV(UIVertex { end   + normal, 0, drawAttr.strokeColor, 0 });
        batch.PushI(0, 1, 2);
        batch.PushI(2, 0, 3);
    }

    void Canvas::DrawLineCap(const Math::fv2& point, const Math::fv2& tangent) {
        Batch batch = NewBatch();
        // no need to draw flat caps: theyre done by default
        switch (drawAttr.drawStyle & UIRender::CAP_MASK) {
            case UIRender::ROUND_CAP: {
                const Math::fv2 tangentR2 = tangent * (Math::ROOT_2 * drawAttr.strokeWeight), normalR2 = tangentR2.Perpend();
                batch.PushV(UIVertex { point - tangentR2, 0, drawAttr.strokeColor, { +1, +1, 0, 0 }, UIRender::CIRCLE });
                batch.PushV(UIVertex { point + normalR2,  0, drawAttr.strokeColor, { -1, +1, 0, 0 }, UIRender::CIRCLE });
                batch.PushV(UIVertex { point - normalR2,  0, drawAttr.strokeColor, { +1, -1, 0, 0 }, UIRender::CIRCLE });
                batch.PushI(0, 1, 2);
                break;
            }
            case UIRender::SQUARE_CAP:  {
                const Math::fv2 t = tangent * drawAttr.strokeWeight;
                const Math::fv2 normal = t.Perpend(), left = point + normal, right = point - normal;
                batch.PushV(UIVertex { left,      0, drawAttr.strokeColor, 0 });
                batch.PushV(UIVertex { left  - t, 0, drawAttr.strokeColor, 0 });
                batch.PushV(UIVertex { right - t, 0, drawAttr.strokeColor, 0 });
                batch.PushV(UIVertex { right,     0, drawAttr.strokeColor, 0 });
                batch.PushI(0, 1, 2);
                batch.PushI(2, 0, 3);
                break;
            }
            default:;
        }
    }

    void Canvas::DrawLineJoin(const Math::fv2& point, const Math::fv2& tangent1, const Math::fv2& tangent2) {
        const float cos = tangent1.Dot(tangent2), sin = tangent1.Cross(tangent2);
        const bool left = sin >= 0;
        const Math::fv2 normal1 = tangent1.Perpend(!left) * drawAttr.strokeWeight,
                        normal2 = tangent2.Perpend(!left) * drawAttr.strokeWeight;
        // when cos : [0,1], angle is small, join is minor arc
        // when cos : [-1,0], angle is large, join is major arc
        Batch batch = NewBatch();

        // bevel joint by default. all joints use this
        batch.PushV(UIVertex { point, 0, drawAttr.strokeColor, 0 });
        batch.PushV(UIVertex { point + normal1, 0, drawAttr.strokeColor, 0 });
        batch.PushV(UIVertex { point + normal2, 0, drawAttr.strokeColor, 0 });
        batch.PushI(0, 1, 2);

        switch (drawAttr.drawStyle & UIRender::JOIN_MASK) {
            case UIRender::ROUND_JOIN: {
                // halven the angles first
                const float u = Math::HALF_ROOT_2 * std::sqrt(1 + cos),
                            v = Math::HALF_ROOT_2 * std::sqrt(1 - cos);
                DrawHalfCircularChord(
                    point,
                    drawAttr.strokeWeight * tangent1.ComplexMul(v, left ? -u : u),
                    u, v, drawAttr.strokeColor);
                break;
            }
            case UIRender::MITER_JOIN: {
                if (cos > -0.9375f) { // miter join will be too large - only render it when cos is reasonable
                    const float h = (cos - 1) / sin;
                    const Math::fv2 outward = (left ? -drawAttr.strokeWeight : drawAttr.strokeWeight) * tangent1.ComplexMul(h, 1);
                    batch.PushV(UIVertex { point + outward, 0, drawAttr.strokeColor, 0 });
                    batch.PushI(1, 3, 2);
                }
                break;
            }
            default:;
        }
    }

    void Canvas::DrawHalfCircularChord(const Math::fv2& center, const Math::fv2& forward, float cos, float sin, const Math::fColor& color) {
        // a v^2 + b v + c + d u + e u v
        static constexpr float HORIZ_COEFF[5] = {
            0.115607368042f,
           -0.356381124853f,
            0.241706294943f,
            1.17272151222f,
            0.0595057818955f,
        };
        // half of cos + isin: u + vi, quarter: s + ti
        const float horiz  = (HORIZ_COEFF[0] * cos + HORIZ_COEFF[1] + HORIZ_COEFF[4] * sin) * cos + HORIZ_COEFF[3] * sin + HORIZ_COEFF[2],
                    vert   = Math::ROOT_2 / std::sqrt(1 + cos);
        // horiz was supposed to be sec(x/2 + pi/4) - sin(x) tan(x/2 + pi/4),
        // but computationally at pi/2 there's a pole (nan) and there's no good way to solve it
        // banged my head against a wall for an hour with no solution in sight,
        // trignonometry ingrained into my brain at this point
        // ...
        // so i used a regression model a v^2 + b v + c + d u + e u v with desmos
        // with error < 0.0004831, using no sqrts or division, just multiplication
        // and also quite fast too, so I'm pretty happy about it.
        // ...
        // i wasted a whole day figuring this out
        // ...
        // shut up
        Batch batch = NewBatch();
        batch.PushV(UIVertex { center + forward * vert,                  0, color, { vert,     0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { center + forward.ComplexMul(cos, +horiz), 0, color, { cos, +horiz, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { center + forward.ComplexMul(cos, -horiz), 0, color, { cos, -horiz, 0, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
    }

    void Canvas::DrawHalfCircle(const Math::fv2& center, const Math::fv2& forward, const Math::fColor& color) {
        Batch batch = NewBatch();
        static constexpr float R2 = Math::ROOT_2;
        const Math::fv2 f = R2 * forward;
        batch.PushV(UIVertex { center + f,                0, color, { +R2, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { center + f.PerpendLeft(),  0, color, { 0, +R2, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { center + f.PerpendRight(), 0, color, { 0, -R2, 0, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
    }

    void Canvas::DrawSimpleCircle(const Math::fv2& center, float radius, const Math::fColor& color) {
        Batch batch = NewBatch();
        static constexpr float R2 = Math::ROOT_2;
        const float diagonal = R2 * radius;
        batch.PushV(UIVertex { { center.x + diagonal, center.y }, 0, color, { +R2, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x, center.y + diagonal }, 0, color, { 0, +R2, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x - diagonal, center.y }, 0, color, { -R2, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x, center.y - diagonal }, 0, color, { 0, -R2, 0, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
        batch.PushI(2, 0, 3);
    }

    void Canvas::DrawQuarterChord(const Math::fv2& center, const Math::fv2& rightMostSide, const Math::fColor& color) {
        Batch batch = NewBatch();
        const Math::fv2 leftMostSide = rightMostSide.PerpendLeft(), right = center + rightMostSide;
        batch.PushV(UIVertex { right,                 0, color, { 1, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { center + leftMostSide, 0, color, { 0, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { right  + leftMostSide, 0, color, { 1, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
    }

    void Canvas::DrawSimpleEllipse(const Math::fv2& center, const Math::fv2& radii, const Math::fColor& color) {
        Batch batch = NewBatch();
        batch.PushV(UIVertex { { center.x - radii.x, center.y - radii.y }, 0, color, { -1, -1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x + radii.x, center.y - radii.y }, 0, color, { +1, -1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x - radii.x, center.y + radii.y }, 0, color, { -1, +1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x + radii.x, center.y + radii.y }, 0, color, { +1, +1, 0, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
        batch.PushI(2, 1, 3);
    }

    void Canvas::DrawSimpleEllipse(const Math::fRect2D& bounds, const Math::fColor& color) {
        Batch batch = NewBatch();
        batch.PushV(UIVertex { { bounds.min.x, bounds.min.y }, 0, color, { -1, -1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { bounds.max.x, bounds.min.y }, 0, color, { +1, -1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { bounds.min.x, bounds.max.y }, 0, color, { -1, +1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { bounds.max.x, bounds.max.y }, 0, color, { +1, +1, 0, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
        batch.PushI(2, 1, 3);
    }

    void Canvas::DrawCircleOutline(const Math::fv2& center, float radius, float thickness, const Math::fColor& color) {
        Batch batch = NewBatch();
        static constexpr float R2 = Math::ROOT_2;
        const float diagonal = R2 * (radius + thickness);
        float inner = (radius - thickness) / (radius + thickness);
        inner *= inner;
        batch.PushV(UIVertex { { center.x + diagonal, center.y }, 0, color, { +R2, 0, inner, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x, center.y + diagonal }, 0, color, { 0, +R2, inner, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x - diagonal, center.y }, 0, color, { -R2, 0, inner, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { center.x, center.y - diagonal }, 0, color, { 0, -R2, inner, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
        batch.PushI(2, 0, 3);
    }

    void Canvas::DrawQuarterArc(const Math::fv2& center, const Math::Rotor2D& start, float radius, float thickness, const Math::fColor& color) {
        const float innerRadius = radius - thickness,
                    innerUV = innerRadius / (radius + thickness), innerSq = innerUV * innerUV;
        Batch batch = NewBatch();
        const Math::fv2 innerForward = start.IHat() * innerRadius,
                        outerForward = start.IHat() * (Math::ROOT_2 * (radius + thickness));
        batch.PushV({ center + innerForward,               0, color, { innerUV, 0, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushV({ center + innerForward.PerpendLeft(), 0, color, { 0, innerUV, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushV({ center + outerForward,               0, color, { Math::ROOT_2, 0, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushV({ center + outerForward.PerpendLeft(), 0, color, { 0, Math::ROOT_2, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2);
        batch.PushI(2, 1, 3);
    }

    void Canvas::DrawCircularArcCCW(const Math::fv2& center, const Math::Rotor2D& mid, Math::Rotor2D step, float radius, float thickness, const Math::fColor& color) {
        const float innerRadius = radius - thickness;
        if (step.Cos() < 0) {
            DrawQuarterArc(center, mid.RotateByInv(step), radius, thickness, color);
            step = step.RotateCW90();
            DrawQuarterArc(center, mid.RotateBy(step), radius, thickness, color);
        }
        const float secant = Math::ROOT_2 / std::sqrt(1 + step.Cos()), outerRadius = secant * (radius + thickness),
                    innerUV = innerRadius / (radius + thickness), innerSq = innerUV * innerUV;
        Batch batch = NewBatch();
        Math::fv2 fwd = mid.IHat();
        batch.PushV({ center + fwd * innerRadius, 0, color, { innerUV, 0, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushV({ center + fwd * outerRadius, 0, color, { secant,  0, innerSq, 0 }, UIRender::CIRCLE });
        fwd = fwd.RotateBy(step);
        const Math::fv2 iUV = step.IHat() * innerUV, sUV = step.IHat() * secant;
        batch.PushV({ center + fwd * innerRadius, 0, color, { iUV.x, iUV.y, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushV({ center + fwd * outerRadius, 0, color, { sUV.x, sUV.y, innerSq, 0 }, UIRender::CIRCLE });
        fwd = step.InvRotate(mid.IHat());
        batch.PushV({ center + fwd * innerRadius, 0, color, { iUV.x, -iUV.y, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushV({ center + fwd * outerRadius, 0, color, { sUV.x, -sUV.y, innerSq, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2); batch.PushI(2, 1, 3);
        batch.PushI(0, 5, 1); batch.PushI(5, 0, 4);
    }

    void Canvas::DrawSimpleRect(const Math::fRect2D& rect, const Math::fColor& color) {
        Batch batch = NewBatch();
        batch.PushV(UIVertex { rect.Corner({ false, false }), 0, color, 0 });
        batch.PushV(UIVertex { rect.Corner({ false, true  }), 0, color, 0 });
        batch.PushV(UIVertex { rect.Corner({ true,  false }), 0, color, 0 });
        batch.PushV(UIVertex { rect.Corner({ true,  true  }), 0, color, 0 });
        batch.PushI(0, 1, 2);
        batch.PushI(2, 1, 3);
    }

    void Canvas::DrawSimpleRoundedRect(const Math::fRect2D& inner, float radius, const Math::fColor& color) {
        const Math::fRect2D outer = inner.Extrude(radius);

        Batch batch = NewBatch();
        batch.PushV(UIVertex { { inner.min.x, outer.min.y }, 0, color, { 1, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.min.x, inner.min.y }, 0, color, { 0, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.min.x, outer.min.y }, 0, color, { 1, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { inner.max.x, outer.min.y }, 0, color, { 1, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.max.x, inner.min.y }, 0, color, { 0, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.max.x, outer.min.y }, 0, color, { 1, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { inner.min.x, outer.max.y }, 0, color, { 1, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.min.x, inner.max.y }, 0, color, { 0, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.min.x, outer.max.y }, 0, color, { 1, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { inner.max.x, outer.max.y }, 0, color, { 1, 0, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.max.x, inner.max.y }, 0, color, { 0, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushV(UIVertex { { outer.max.x, outer.max.y }, 0, color, { 1, 1, 0, 0 }, UIRender::CIRCLE });
        batch.PushI(0, 1, 2); batch.PushI(3, 4, 5); batch.PushI(6, 7, 8); batch.PushI(9, 10, 11);

        batch.PushV(UIVertex { { inner.min.x, outer.min.y }, 0, color, 0 });
        batch.PushV(UIVertex { { inner.max.x, outer.min.y }, 0, color, 0 });
        batch.PushV(UIVertex { { outer.min.x, inner.min.y }, 0, color, 0 });
        batch.PushV(UIVertex { { outer.max.x, inner.min.y }, 0, color, 0 });
        batch.PushV(UIVertex { { outer.min.x, inner.max.y }, 0, color, 0 });
        batch.PushV(UIVertex { { outer.max.x, inner.max.y }, 0, color, 0 });
        batch.PushV(UIVertex { { inner.min.x, outer.max.y }, 0, color, 0 });
        batch.PushV(UIVertex { { inner.min.x, outer.max.y }, 0, color, 0 });
        batch.PushI(12, 13, 14); batch.PushI(14, 13, 15);
        batch.PushI(14, 15, 16); batch.PushI(16, 15, 17);
        batch.PushI(16, 17, 18); batch.PushI(18, 17, 19);
    }

    void Canvas::DrawRectStroke(const Math::fRect2D& rect) {
        const Math::fRect2D outer = rect.Extrude(drawAttr.strokeWeight),
                            inner = rect.Inset(drawAttr.strokeWeight);

        Batch batch = NewBatch();
        switch (drawAttr.drawStyle & UIRender::JOIN_MASK) {
            case UIRender::ROUND_JOIN:
                // just add the corners, the rest will be done by BEVEL_JOIN
                DrawQuarterChord(rect.min,                   { -drawAttr.strokeWeight, 0 }, drawAttr.strokeColor);
                DrawQuarterChord({ rect.max.x, rect.min.y }, { 0, -drawAttr.strokeWeight }, drawAttr.strokeColor);
                DrawQuarterChord({ rect.min.x, rect.max.y }, { 0, +drawAttr.strokeWeight }, drawAttr.strokeColor);
                DrawQuarterChord(rect.max,                   { +drawAttr.strokeWeight, 0 }, drawAttr.strokeColor);
                batch.Refresh();
                [[fallthrough]];
            case UIRender::BEVEL_JOIN: {
                /*
                 *     E          G
                 *   J              L
                 *        F    H
                 *
                 *        B    D
                 *   I              K
                 *     A          C
                 */
                // top and bottom rows
                batch.PushV({ { rect.min.x,  outer.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.min.x, inner.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { rect.max.x,  outer.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.max.x, inner.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushI(0, 1, 2); batch.PushI(2, 1, 3); // ABCD

                batch.PushV({ { rect.min.x,  outer.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.min.x, inner.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { rect.max.x,  outer.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.max.x, inner.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushI(4, 5, 6); batch.PushI(6, 5, 7); // EFGH

                batch.PushV({ { outer.min.x, rect.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { outer.min.x, rect.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { outer.max.x, rect.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { outer.max.x, rect.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushI(0, 8, 1); batch.PushI(8, 9, 1);
                batch.PushI(9, 5, 1); batch.PushI(9, 4, 5); // AIB IJB JFB JEF
                batch.PushI(3, 10, 2); batch.PushI(3, 11, 10);
                batch.PushI(3, 7, 11); batch.PushI(7, 6, 11); // DKC DLK DHL HGL
                break;
            }
            case UIRender::MITER_JOIN: {
                // top and bottom rows
                batch.PushV({ { inner.min.x, outer.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.min.x, inner.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.max.x, outer.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.max.x, inner.min.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushI(0, 1, 2); batch.PushI(2, 1, 3);

                batch.PushV({ { inner.min.x, outer.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.min.x, inner.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.max.x, outer.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushV({ { inner.max.x, inner.max.y }, 0, drawAttr.strokeColor, 0 });
                batch.PushI(4, 5, 6); batch.PushI(6, 5, 7);

                batch.PushV({ outer.min,                    0, drawAttr.strokeColor, 0 }); // 8
                batch.PushV({ { outer.max.x, outer.min.y }, 0, drawAttr.strokeColor, 0 }); // 9
                batch.PushV({ { outer.min.x, outer.max.y }, 0, drawAttr.strokeColor, 0 }); // 10
                batch.PushV({ outer.max,                    0, drawAttr.strokeColor, 0 }); // 11
                batch.PushI(0, 10, 8); batch.PushI(0, 4, 10);
                batch.PushI(2, 9, 11); batch.PushI(2, 11, 6);
                break;
            }
            default:;
        }
    }

    void Canvas::Path::AddPoint(const Math::fv2& point) {
        if (NoPointsYet()) return BeginLineSegment(point);

        const Math::fv2 tangent = lastPoint.DirTowards(point);
        canvas.DrawSimpleLine(lastPoint, point, tangent);
        EndLineSegment(point, tangent);
    }

    bool Canvas::Path::NoPointsYet() {
        return closing == OPEN_CURVE || closing == CLOSED_CURVE;
    }

    void Canvas::Path::BeginLineSegment(const Math::fv2& p) {
        switch (closing) {
            case OPEN_CURVE:
                // no points yet, encountered first point
                lastPoint = p;
                closing = OPEN_CURVE_SECOND_POINT;
                break;
            case CLOSED_CURVE:
                // no points yet, but we need to store first point to close the curve
                firstPoint = lastPoint = p;
                closing = CLOSED_CURVE_SECOND_POINT;
                break;
            default:;
        }
    }

    void Canvas::Path::EndLineSegment(const Math::fv2& p, const Math::fv2& tangent) {
        switch (closing) {
            case OPEN_CURVE_SECOND_POINT: {
                // we cap of the first point since we just left it
                canvas.DrawLineCap(lastPoint, tangent);
                lastPoint = p;
                lastTangent = tangent;
                closing = OPEN_CURVE_MIDDLE_POINT;
                break;
            }
            case OPEN_CURVE_MIDDLE_POINT: {
                // since we're in the middle, we now have to draw joins instead of caps
                canvas.DrawLineJoin(lastPoint, lastTangent, tangent);
                lastPoint = p;
                lastTangent = tangent;
                break;
            }
            case CLOSED_CURVE_SECOND_POINT: {
                // we cant draw the join of the previous point, since we dont know the last
                lastPoint = p;
                firstTangent = lastTangent = tangent;
                closing = CLOSED_CURVE_MIDDLE_POINT;
                break;
            }
            case CLOSED_CURVE_MIDDLE_POINT: {
                // now we can draw the previous join
                canvas.DrawLineJoin(lastPoint, lastTangent, tangent);
                lastPoint = p;
                lastTangent = tangent;
                break;
            }
            default:;
        }
    }

    void Canvas::Path::DrawCircularArcCCW(const Math::fv2& startPoint, const Math::fv2& center, const Math::Rotor2D& turn) {
        const Math::Rotor2D halfTurn = turn.HalvedCCW();
        const auto [direction, radius] = (startPoint - center).NormAndLen();
        const Math::Rotor2D middle = Math::Rotor2D::FromComplex({ direction.x, direction.y }) + halfTurn;
        canvas.DrawCircularArcCCW(center, middle, halfTurn, radius, canvas.drawAttr.strokeWeight, canvas.drawAttr.strokeColor);
    }

    void Canvas::Path::AddCircularArc(const Math::fv2& center, const Math::Rotor2D& turn, ArcDirection dir) {
        if (NoPointsYet()) return;

        const Math::fv2 lastDirection = (lastPoint - center), endPoint = lastDirection.RotateBy(turn) + center;
        const bool swap = (dir & 1) ^ ((dir & 2) && turn.Sin() < 0);
        DrawCircularArcCCW(swap ? endPoint : lastPoint, center, swap ? turn.Inverse() : turn);

        EndLineSegment(endPoint, lastDirection.Norm().Perpend(!swap));
        lastTangent = lastTangent.RotateBy(turn);
    }

    Canvas::Path::~Path() {
        // end the path
        switch (closing) {
            case OPEN_CURVE: case CLOSED_CURVE:
                // no path was drawn. only 1 point.
                return;
            case OPEN_CURVE_SECOND_POINT: case OPEN_CURVE_MIDDLE_POINT:
                // this is the last point. cap it off
                canvas.DrawLineCap(lastPoint, -lastTangent);
                break;
            case CLOSED_CURVE_SECOND_POINT: case CLOSED_CURVE_MIDDLE_POINT: {
                // this is the last point. close it off with the first
                const Math::fv2 tangent = lastPoint.DirTowards(firstPoint);
                canvas.DrawLineJoin(lastPoint, lastTangent, tangent); // last join
                canvas.DrawSimpleLine(lastPoint, firstPoint, tangent);
                canvas.DrawLineJoin(firstPoint, tangent, firstTangent); // first join
                break;
            }
            default:;
        }
    }

    void Canvas::StrokeWeight(float weight) {
        drawAttr.strokeWeight = weight;
        drawAttr.drawStyle &= ~UIRender::NO_STROKE;
    }

    void Canvas::StrokeCap(UIRender::RenderStyle cap) {
        drawAttr.drawStyle &= ~UIRender::CAP_MASK;
        drawAttr.drawStyle |= cap;
    }

    void Canvas::StrokeJoin(UIRender::RenderStyle join) {
        drawAttr.drawStyle &= ~UIRender::JOIN_MASK;
        drawAttr.drawStyle |= join;
    }

    void Canvas::Fill(const Math::fColor& fillColor) {
        drawAttr.fillColor = fillColor;
        drawAttr.drawStyle &= ~UIRender::NO_FILL;
    }

    void Canvas::Stroke(const Math::fColor& strokeColor) {
        drawAttr.strokeColor = strokeColor;
        drawAttr.drawStyle &= ~UIRender::NO_STROKE;
    }

    void Canvas::NoStroke() {
        drawAttr.drawStyle |= UIRender::NO_STROKE;
    }

    void Canvas::NoFill() {
        drawAttr.drawStyle |= UIRender::NO_FILL;
    }

    Canvas::PushStyles::PushStyles(Canvas& canvas) : canvas(canvas), originalAttr(canvas.drawAttr) {}
    Canvas::PushStyles::~PushStyles() {
        canvas.drawAttr = originalAttr;
    }

    Math::fv2 Canvas::TransformToWorldSpace(const Math::fv2& point) const {
        return modelTransform * point + modelTranslate;
    }

    void Canvas::ApplyMatrix(const Math::Matrix2x2& matrix) {
        modelTransform = matrix * modelTransform;
    }

    void Canvas::ResetMatrix() {
        modelTransform = Math::Matrix2x2::Identity();
    }

    void Canvas::Rotate(const Math::Rotor2D& rotor) {
        modelTransform.RotateByLin(rotor);
    }

    void Canvas::Scale(const Math::fv2& scale) {
        modelTransform.ScaleByLinear(scale);
    }

    void Canvas::RotateAround(const Math::Rotor2D& rotor, const Math::fv2& origin) {
        modelTransform.RotateByLin(rotor);
        modelTranslate += origin - origin.RotateBy(rotor);
    }

    void Canvas::ScaleAround(const Math::fv2& scale, const Math::fv2& origin) {
        modelTransform.ScaleByLinear(scale);
        modelTranslate += origin - origin * scale;
    }

    void Canvas::Shear(const Math::fv2& shear) {
        modelTransform = Math::Matrix2x2::FromColumns({
            modelTransform[0] + shear.y * modelTransform[1],
            modelTransform[1] + shear.x * modelTransform[0],
        });
    }

    void Canvas::Translate(const Math::fv2& translate) {
        modelTranslate += translate;
    }

    void Canvas::BeginFrame() {
        renderCanvas.BeginContext();
    }

    void Canvas::EndFrame() {
        renderCanvas.EndContext();
        renderCanvas.DrawContext();
    }
}
