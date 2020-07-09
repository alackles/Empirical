#ifndef EMP_LOADING_MODAL_H
#define EMP_LOADING_MODAL_H

/** 
 * To add a loading modal to your web page, you must 
 * link the LoadingModal.js script directly after the 
 * body tag in the HTML doc. To close the script, call 
 * CloseLoadingModal() at the end of your .cc file, or
 * at the point which you wish to loading modal to disappear.
 * 
 * See https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo
 * for more details.
*/
#include "../web/Element.h"
#include "../web/Div.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"

namespace emp {
    namespace prefab{
        void CloseLoadingModal(){
        emscripten_run_script("CloseLoadingModal();"); 
        }
    }
}

#endif