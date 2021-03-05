add_library(frontend-common
  common_host_interface.cpp
  common_host_interface.h
  controller_interface.cpp
  controller_interface.h
  cubeb_audio_stream.cpp
  cubeb_audio_stream.h
  fullscreen_ui.cpp
  fullscreen_ui.h
  fullscreen_ui_progress_callback.cpp
  fullscreen_ui_progress_callback.h
  game_list.cpp
  game_list.h
  game_settings.cpp
  game_settings.h
  icon.cpp
  icon.h
  ini_settings_interface.cpp
  ini_settings_interface.h
  imgui_fullscreen.h
  imgui_fullscreen.cpp
  imgui_impl_opengl3.cpp
  imgui_impl_opengl3.h
  imgui_impl_vulkan.cpp
  imgui_impl_vulkan.h
  imgui_styles.cpp
  imgui_styles.h  
  opengl_host_display.cpp
  opengl_host_display.h
  postprocessing_chain.cpp
  postprocessing_chain.h
  postprocessing_shader.cpp
  postprocessing_shader.h
  postprocessing_shadergen.cpp
  postprocessing_shadergen.h
  vulkan_host_display.cpp
  vulkan_host_display.h
)

target_link_libraries(frontend-common PUBLIC core common glad vulkan-loader cubeb imgui simpleini tinyxml2 scmversion)

if(WIN32)
  target_sources(frontend-common PRIVATE
    d3d11_host_display.cpp
    d3d11_host_display.h
  )
  target_link_libraries(frontend-common PRIVATE d3d11.lib dxgi.lib)
endif()

if(NOT BUILD_LIBRETRO_CORE)
  target_sources(frontend-common PRIVATE
    imgui_impl_opengl3.h
    imgui_impl_opengl3.cpp
  )

  target_sources(frontend-common PRIVATE
    imgui_impl_vulkan.h
    imgui_impl_vulkan.cpp
  )

  if(WIN32)
    target_sources(frontend-common PRIVATE
      imgui_impl_dx11.h
      imgui_impl_dx11.cpp
    )
  endif()

  if(USE_EVDEV)
    find_package(LIBEVDEV REQUIRED)
    target_compile_definitions(frontend-common PUBLIC "-DWITH_EVDEV=1")
    target_include_directories(frontend-common PRIVATE ${LIBEVDEV_INCLUDE_DIRS})
    target_link_libraries(frontend-common PRIVATE ${LIBEVDEV_LIBRARIES})
    target_sources(frontend-common PRIVATE
      evdev_controller_interface.cpp
      evdev_controller_interface.h
    )
  endif()

  if(ENABLE_DISCORD_PRESENCE)
    target_compile_definitions(frontend-common PUBLIC -DWITH_DISCORD_PRESENCE=1)
    target_link_libraries(frontend-common PRIVATE discord-rpc)
  endif()

  if(ENABLE_CHEEVOS)
    target_sources(frontend-common PRIVATE
      http_downloader.cpp
      http_downloader.h
    )
    if(WIN32)
      target_sources(frontend-common PRIVATE
        http_downloader_winhttp.cpp
        http_downloader_winhttp.h
      )
    endif()

    target_sources(frontend-common PRIVATE
      cheevos.cpp
      cheevos.h
    )
    target_compile_definitions(frontend-common PUBLIC -DWITH_CHEEVOS=1)
    target_link_libraries(frontend-common PRIVATE rcheevos rapidjson)
  endif()

  # Copy the provided data directory to the output directory.
  add_custom_command(TARGET frontend-common POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
  )
endif()

