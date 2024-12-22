#pragma once
#include <utility>

#include "VertexElement.h"

namespace Quasi::Graphics::MeshUtils {
    template <class T>
    struct OptionsFor {};
    
    template <class R, class MD> concept MTransformer =
        std::is_same_v<VertexBuilder::MeshConstructData2D, MD> && Math::ITransformer2D<R> ||
        std::is_same_v<VertexBuilder::MeshConstructData3D, MD> && Math::ITransformer3D<R>;

    template <class T>
    struct MeshConstructor {
        using Options = OptionsFor<T>;
        using MData = typename Options::MData;
        
        template <class F> using ResultingV = decltype(std::declval<F>()(MData {}));

        template <class F>
        static void Merge(const Options& options, F&& f, Mesh<ResultingV<F>>& out) {
            return (T { options }).MergeImpl(std::forward<F>(f), out);
        }

        template <class F>
        static void Merge(F&& f, Mesh<ResultingV<F>>& out) {
            return (T { Options {} }).MergeImpl(std::forward<F>(f), out);
        }

        template <class F>
        static void Merge(const Options& options, F&& f, const MTransformer<MData> auto& transform, Mesh<ResultingV<F>>& out) {
            return Merge(options, [&] (const MData& data) { return VertexMul(std::forward<F>(f)(data), transform); }, out);
        }

        template <class F>
        static void Merge(F&& f, const MTransformer<MData> auto& transform, Mesh<ResultingV<F>>& out) {
            return Merge({}, [&] (const MData& data) { return VertexMul(std::forward<F>(f)(data), transform); }, out);
        }

        template <class F>
        static auto Create(const Options& options, F&& f) {
            Mesh<ResultingV<F>> out;
            Merge(options, std::forward<F>(f), out);
            return out;
        }

        template <class F>
        static auto Create(F&& f) {
            return Create(Options {}, std::forward<F>(f));
        }

        template <class F>
        static auto Create(const Options& options, F&& f, const MTransformer<MData> auto& transform) {
            Mesh<ResultingV<F>> out;
            Merge(options, std::forward<F>(f), transform, out);
            return out;
        }

        template <class F>
        static auto Create(F&& f, const MTransformer<MData> auto& transform) {
            return Create({}, std::forward<F>(f), transform);
        }

        template <class F>
        auto operator()(const Options& options, F&& f) const {
            return Create(options, f);
        }

        template <class F>
        auto operator()(F&& f) const {
            return Create(f);
        }

        template <class F>
        auto operator()(const Options& options, F&& f, const MTransformer<MData> auto& transform) const {
            return Create(options, f, transform);
        }

        template <class F>
        auto operator()(F&& f, const MTransformer<MData> auto& transform) const {
            return Create(f, transform);
        }
    };
}
