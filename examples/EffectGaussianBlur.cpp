/*
 * Copyright (c) 2024 the ThorVG project. All rights reserved.

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

#include "Example.h"

/************************************************************************/
/* ThorVG Drawing Contents                                              */
/************************************************************************/

struct UserExample : tvgexam::Example
{
    tvg::Scene* scene1 = nullptr;   //direction both
    tvg::Scene* scene2 = nullptr;   //direction horizontal
    tvg::Scene* scene3 = nullptr;   //direction vertical

    bool content(tvg::Canvas* canvas, uint32_t w, uint32_t h) override
    {
        if (!canvas) return false;

        //Prepare a scene for post effects (direction both)
        {
            scene1 = tvg::Scene::gen();

            auto picture = tvg::Picture::gen();
            picture->load(EXAMPLE_DIR"/svg/tiger.svg");
            picture->size(w / 2, h / 2);

            scene1->push(picture);
            canvas->push(scene1);
        }

        //Prepare a scene for post effects (direction horizontal)
        {
            scene2 = tvg::Scene::gen();

            auto picture = tvg::Picture::gen();
            picture->load(EXAMPLE_DIR"/svg/tiger.svg");
            picture->size(w / 2, h / 2);
            picture->translate(w / 2, 0);

            scene2->push(picture);
            canvas->push(scene2);
        }

        //Prepare a scene for post effects (direction vertical)
        {
            scene3 = tvg::Scene::gen();

            auto picture = tvg::Picture::gen();
            picture->load(EXAMPLE_DIR"/svg/tiger.svg");
            picture->size(w / 2, h / 2);
            picture->translate(0, h / 2);

            scene3->push(picture);
            canvas->push(scene3);
        }


        return true;
    }

    bool update(tvg::Canvas* canvas, uint32_t elapsed) override
    {
        if (!canvas) return false;

        canvas->clear(false);

        auto progress = tvgexam::progress(elapsed, 2.5f, true);   //2.5 seconds

        //Clear the previously applied effects
        scene1->push(tvg::SceneEffect::ClearAll);
        //Apply GaussianBlur post effect (sigma, direction, border option, quality)
        scene1->push(tvg::SceneEffect::GaussianBlur, 10.0f * progress, 0, 0, 100);

        scene2->push(tvg::SceneEffect::ClearAll);
        scene2->push(tvg::SceneEffect::GaussianBlur, 10.0f * progress, 1, 0, 100);

        scene3->push(tvg::SceneEffect::ClearAll);
        scene3->push(tvg::SceneEffect::GaussianBlur, 10.0f * progress, 2, 0, 100);

        canvas->update();

        return true;
    }

};


/************************************************************************/
/* Entry Point                                                          */
/************************************************************************/

int main(int argc, char **argv)
{
    return tvgexam::main(new UserExample, argc, argv, 800, 800, 4, true);
}