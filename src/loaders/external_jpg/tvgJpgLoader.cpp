/*
 * Copyright (c) 2021 - 2023 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <memory.h>
#include <turbojpeg.h>
#include "tvgJpgLoader.h"

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

void JpgLoader::clear()
{
    if (freeData) free(data);
    data = nullptr;
    size = 0;
    freeData = false;
}


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

JpgLoader::JpgLoader() : LoadModule(FileType::Jpg)
{
    jpegDecompressor = tjInitDecompress();
}


JpgLoader::~JpgLoader()
{
    clear();
    tjDestroy(jpegDecompressor);

    //This image is shared with raster engine.
    tjFree(image);
}


bool JpgLoader::open(const string& path)
{
    auto jpegFile = fopen(path.c_str(), "rb");
    if (!jpegFile) return false;

    auto ret = false;

    //determine size
    if (fseek(jpegFile, 0, SEEK_END) < 0) goto finalize;
    if (((size = ftell(jpegFile)) < 1)) goto finalize;
    if (fseek(jpegFile, 0, SEEK_SET)) goto finalize;

    data = (unsigned char *) malloc(size);
    if (!data) goto finalize;

    freeData = true;

    if (fread(data, size, 1, jpegFile) < 1) goto failure;

    int width, height, subSample, colorSpace;
    if (tjDecompressHeader3(jpegDecompressor, data, size, &width, &height, &subSample, &colorSpace) < 0) {
        TVGERR("JPG LOADER", "%s", tjGetErrorStr());
        goto failure;
    }

    w = static_cast<float>(width);
    h = static_cast<float>(height);
    cs = ColorSpace::ARGB8888;
    ret = true;

    goto finalize;

failure:
    clear();

finalize:
    fclose(jpegFile);
    return ret;
}


bool JpgLoader::open(const char* data, uint32_t size, TVG_UNUSED const string& rpath, bool copy)
{
    int width, height, subSample, colorSpace;
    if (tjDecompressHeader3(jpegDecompressor, (unsigned char *) data, size, &width, &height, &subSample, &colorSpace) < 0) return false;

    if (copy) {
        this->data = (unsigned char *) malloc(size);
        if (!this->data) return false;
        memcpy((unsigned char *)this->data, data, size);
        freeData = true;
    } else {
        this->data = (unsigned char *) data;
        freeData = false;
    }

    w = static_cast<float>(width);
    h = static_cast<float>(height);
    cs = ColorSpace::ARGB8888;
    this->size = size;

    return true;
}


bool JpgLoader::read()
{
    if (!LoadModule::read()) return true;

    if (w == 0 || h == 0) return false;

    /* OPTIMIZE: We assume the desired colorspace is ColorSpace::ARGB
       How could we notice the renderer colorspace at this time? */
    if (image) tjFree(image);
    image = (unsigned char *)tjAlloc(static_cast<int>(w) * static_cast<int>(h) * tjPixelSize[TJPF_BGRX]);
    if (!image) return false;

    //decompress jpg image
    if (tjDecompress2(jpegDecompressor, data, size, image, static_cast<int>(w), 0, static_cast<int>(h), TJPF_BGRX, 0) < 0) {
        TVGERR("JPG LOADER", "%s", tjGetErrorStr());
        tjFree(image);
        image = nullptr;
        return false;
    }

    clear();
    return true;
}


unique_ptr<Surface> JpgLoader::bitmap()
{
    if (!image) return nullptr;

    //TODO: It's better to keep this surface instance in the loader side
    auto surface = new Surface;
    surface->buf8 = image;
    surface->stride = w;
    surface->w = w;
    surface->h = h;
    surface->cs = cs;
    surface->channelSize = sizeof(uint32_t);
    surface->premultiplied = true;
    surface->owner = true;

    return unique_ptr<Surface>(surface);
}
