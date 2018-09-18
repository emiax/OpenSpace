##########################################################################################
#                                                                                        #
# OpenSpace                                                                              #
#                                                                                        #
# Copyright (c) 2014-2018                                                                #
#                                                                                        #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this   #
# software and associated documentation files (the "Software"), to deal in the Software  #
# without restriction, including without limitation the rights to use, copy, modify,     #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to     #
# permit persons to whom the Software is furnished to do so, subject to the following    #
# conditions:                                                                            #
#                                                                                        #
# The above copyright notice and this permission notice shall be included in all copies  #
# or substantial portions of the Software.                                               #
#                                                                                        #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,    #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A          #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT     #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF   #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE   #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                          #
##########################################################################################

function(build_webgui_frontend_source target_name)
    set(NPM_COMMAND "npm" CACHE STRING "Location of NPM executable unless in PATH.")
    # copy webgui source
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${NPM_COMMAND} run build
        WORKING_DIRECTORY ${WEBGUI_MODULE_PATH}/ext/frontend
        VERBATIM
    )
endfunction()

function(build_webgui_backend_source target_name)
    set(NPM_COMMAND "npm" CACHE STRING "Location of NPM executable unless in PATH.")
    # copy webgui source
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        COMMAND ${NPM_COMMAND} run build
        WORKING_DIRECTORY ${WEBGUI_MODULE_PATH}/ext/backend
        VERBATIM
    )
endfunction()