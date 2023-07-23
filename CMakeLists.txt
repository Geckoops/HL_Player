cmake_minimum_required(VERSION 3.24)

project(HL_Player)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_PREFIX_PATH "C:\\Qt6.2.4\\6.2.4\\msvc2019_64\\lib\\cmake")

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Core5Compat
        Multimedia
        MultimediaWidgets
        OpenGLWidgets
        OpenGL
        Sql
        REQUIRED)

include_directories(include)

set(CMAKE_AUTOUIC_SEARCH_PATHS ./src)
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS ./ui)
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS ./test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

set(FFMPEG_INCLUDE_DIR "C:\\Qt6.2.4\\external_lib\\ffmpeg-master-latest-win64-gpl-shared\\include")
set(FFMPEG_LIB_DIR "C:\\Qt6.2.4\\external_lib\\ffmpeg-master-latest-win64-gpl-shared\\lib")
set(FFMPEG_BIN_DIR "C:\\Qt6.2.4\\external_lib\\ffmpeg-master-latest-win64-gpl-shared\\bin")

add_executable(HL_Player main.cpp
        res/res.qrc
        src/Demux.cpp include/Demux.h
        src/Util.cpp include/Util.h
        include/Reference.h
        src/Decode.cpp include/Decode.h
        src/VideoWidget.cpp include/VideoWidget.h
        src/Play.cpp include/Play.h ui/Play.ui
        src/Resample.cpp include/Resample.h
        src/AudioPlay.cpp include/AudioPlay.h
        src/QtAudioPlay.cpp include/QtAudioPlay.h
        src/AudioThread.cpp include/AudioThread.h
        src/VideoThread.cpp include/VideoThread.h
        include/VideoCall.h
        src/DemuxThread.cpp include/DemuxThread.h
        src/Slider.cpp include/Slider.h
        src/PlayWidget.cpp include/PlayWidget.h
        src/ToolWidget.cpp include/ToolWidget.h
        src/RootWidget.cpp include/RootWidget.h
        src/LogoGLWidget.cpp include/LogoGLWidget.h
        src/TopWidget.cpp include/TopWidget.h ui/TopWidget.ui
        src/RecordListItem.cpp include/RecordListItem.h ui/RecordListItem.ui
        src/HLabel.cpp include/HLabel.h
        src/SubManager.cpp include/SubManager.h
        src/HComboBox.cpp include/HComboBox.h
        src/SqliteHelper.cpp include/SqliteHelper.h)

target_include_directories(HL_Player PRIVATE ${FFMPEG_INCLUDE_DIR})

target_link_directories(HL_Player PRIVATE ${FFMPEG_LIB_DIR})

target_link_libraries(HL_Player
        Qt6::Multimedia
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt6::Core5Compat
        Qt6::OpenGLWidgets
        Qt6::OpenGL
        Qt6::MultimediaWidgets
        Qt6::Sql
        avcodec
        avformat
        avutil
        swscale
        swresample
        avfilter
)

add_executable(Launcher Launcher.cpp)

# add_executable(test
#         test.cpp
#         src/SqliteHelper.cpp
#         include/SqliteHelper.h
# )

# target_include_directories(test PRIVATE ${FFMPEG_INCLUDE_DIR})
# target_link_directories(test PRIVATE ${FFMPEG_LIB_DIR})
# target_include_directories(test PRIVATE test)
# target_link_libraries(test
# Qt6::Multimedia
# Qt6::Sql
# Qt::Core
# Qt::Gui
# Qt::Widgets
# Qt6::Sql
# Qt6::OpenGLWidgets
# Qt6::OpenGL
# libavcodec
# avformat
# avutil
# swscale
# swresample
# )
