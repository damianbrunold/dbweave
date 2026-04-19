/*  DB-WEAVE Qt 6 port - VCL TBitmap compatibility (Phase 1)
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  Minimal TBitmap-compatible wrapper around QImage. Only the subset
    actually used by non-drawing code (bitmap import/export, file I/O
    convenience) is provided. Pixel-level drawing against this type is
    NOT shimmed -- drawing code must be rewritten against QPainter /
    QImage during each module's port.
*/

#ifndef DBWEAVE_COMPAT_TBITMAP_COMPAT_H
#define DBWEAVE_COMPAT_TBITMAP_COMPAT_H

#include <QImage>
#include <QString>

class TBitmap
{
public:
    TBitmap() = default;
    explicit TBitmap(const QImage& _img)
        : image(_img)
    {
    }

    int Width() const
    {
        return image.width();
    }
    int Height() const
    {
        return image.height();
    }

    void SetSize(int _w, int _h)
    {
        image = QImage(_w, _h, QImage::Format_ARGB32);
        image.fill(Qt::white);
    }

    bool LoadFromFile(const QString& _path)
    {
        return image.load(_path);
    }
    bool SaveToFile(const QString& _path, const char* _format = nullptr) const
    {
        return image.save(_path, _format);
    }

    QImage& Image()
    {
        return image;
    }
    const QImage& Image() const
    {
        return image;
    }

private:
    QImage image;
};

#endif /* DBWEAVE_COMPAT_TBITMAP_COMPAT_H */
