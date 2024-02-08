#pragma once
#include "ft2build.h"
#include "NumTypes.h"
#include "opengl.h"

#include FT_FREETYPE_H

namespace Graphics {
    class Font;
    
    class FontDevice {
        FT_Library libHandle = nullptr;
        uint dpi = 96; // default value
    public:
        OPENGL_API FontDevice();
        OPENGL_API ~FontDevice();

        FontDevice(const FontDevice&) = delete;
        FontDevice& operator=(const FontDevice&) = delete;
        static void Transfer(FontDevice& dest, FontDevice&& from);

        void SetDPI(uint newDpi) { dpi = newDpi; }
        
        inline static FontDevice* Instance = nullptr;
        static FT_Library Library() { return Instance->libHandle; }
        static uint DPI() { return Instance->dpi; }

        friend Font;
    };

    struct PointPer64 {
    private:
        int pp64;
        PointPer64(int points, int /* very nice hack to have 2 constructors */) : pp64(points) {}
    public:
        PointPer64(int points) : pp64(points * 64) {}
        PointPer64(float points) : pp64((int)(points * 64.0f)) {}
        static PointPer64 inP64(int pointsPer64) { return { pointsPer64, 0 }; }
        operator int() const { return pp64; }
        int points() const { return pp64 >> 6; }
        float pointsf() const { return (float)pp64 / 64.0f; }
        int pointsPer64() const { return pp64; }
        int px() const { return pp64 * (int)FontDevice::DPI() / (72 * 64); }
        float pxf() const { return (float)(pp64 * FontDevice::DPI()) / (72.0f * 64.0f); }

        float operator/(PointPer64 other) const { return (float)pp64 / (float)other.pp64; }
    };
    
    inline PointPer64 operator ""_p64(uint64 p64) {
        return PointPer64::inP64((int)p64);
    }

    inline PointPer64 operator ""_points(uint64 points) {
        return { (int)points };
    }

    inline PointPer64 operator ""_points(quadf points) {
        return { (float)points };
    }
}
