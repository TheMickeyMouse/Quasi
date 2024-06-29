#pragma once
#include <utility>

#include "Matrix.h"
#include "VertexConverter.h"
#include "VertexElement.h"

namespace Quasi::Graphics::MeshUtils {
    using Vertex2D = Math::fVector2;
    using Vertex3D = VertexNormal3D;

    struct DefaultBuilder {
        Vertex2D operator()(const VertexBuilder::MeshConstructData2D& args) const { return args.Position; }
        Vertex3D operator()(const VertexBuilder::MeshConstructData3D& args) const { return { args.Position, args.Normal }; }
    };

    template <class T>
    struct OptionsFor {};

    template <class T>
    struct MeshConstructor {
        using Options = OptionsFor<T>;

        template <class F>
        static auto Create(const Options& options, F&& f) {
            return T { options }.CreateImpl(std::forward<F>(f));
        }

        template <class F>
        static auto Create(F&& f) {
            return T { Options {} }.CreateImpl(std::forward<F>(f));
        }

        template <class F>
        static auto Create(const Options& options, F&& f, const Math::Matrix3D& transform) {
            return Create(options, std::forward<F>(f)).ApplyTransform(transform);
        }

        template <class F>
        static auto Create(F&& f, const Math::Matrix3D& transform) {
            return Create({}, std::forward<F>(f)).ApplyTransform(transform);
        }

        static auto Create(const Options& options = {}) {
            return Create(options, DefaultBuilder {});
        }

        static auto Create(const Options& options, const Math::Matrix3D& transform) {
            return Create(options, DefaultBuilder {}, transform);
        }

        static auto Create(const Math::Matrix3D& transform) {
            return Create({}, DefaultBuilder {}, transform);
        }

        template <class F>
        [[nodiscard]] auto operator()(const Options& options, F&& f) const {
            return Create(options, f);
        }

        template <class F>
        [[nodiscard]] auto operator()(F&& f) const {
            return Create(f);
        }

        template <class F>
        [[nodiscard]] auto operator()(const Options& options, F&& f, const Math::Matrix3D& transform) const {
            return Create(options, f, transform);
        }

        template <class F>
        [[nodiscard]] auto operator()(F&& f, const Math::Matrix3D& transform) const {
            return Create(f, transform);
        }

        [[nodiscard]] auto operator()(const Options& options = {}) const {
            return Create(options);
        }

        [[nodiscard]] auto operator()(const Options& options, const Math::Matrix3D& transform) const {
            return Create(options, transform);
        }

        [[nodiscard]] auto operator()(const Math::Matrix3D& transform) const {
            return Create(transform);
        }
    };
}
