# Find Python for shader processing
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# List of shader files to process
set(SHADER_FILES
    gfx_shader.glsl
    text_shader.glsl
    text_debug.glsl
    bitmap_shader.glsl
    gfx_debug.glsl
)

# Template file
set(SHADER_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/shader_template.h.in")
set(SHADER_GEN_SCRIPT "${CMAKE_SOURCE_DIR}/tools/cmake_shader_gen.py")

# Process each shader file
foreach(SHADER_FILE ${SHADER_FILES})
    # Get shader name without extension
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
    
    # Input and output paths
    set(SHADER_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}")
    set(SHADER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.h")
    
    # Generate header file from shader
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${SHADER_GEN_SCRIPT} 
                ${SHADER_INPUT} ${SHADER_NAME} ${SHADER_TEMPLATE} ${SHADER_OUTPUT}
        DEPENDS ${SHADER_INPUT} ${SHADER_TEMPLATE} ${SHADER_GEN_SCRIPT}
        COMMENT "Generating ${SHADER_NAME}.h from ${SHADER_FILE}"
        VERBATIM
    )
    
    # Add to list of generated files
    list(APPEND GENERATED_SHADER_HEADERS ${SHADER_OUTPUT})
endforeach()

# Create a custom target that depends on all generated headers
add_custom_target(generate_shaders ALL DEPENDS ${GENERATED_SHADER_HEADERS})

# Make the generated headers directory available to parent scope
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
