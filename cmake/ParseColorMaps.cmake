function(derive_brightness base_color brightness new_color)
    if ((brightness LESS 0) OR (brightness GREATER 3))
        message(FATAL_ERROR "Incorrect brightness ${brightness}")
    endif ()
    set(coeff 135)
    if (brightness EQUAL 0)
        set(coeff 180)
    elseif (brightness EQUAL 1)
        set(coeff 220)
    elseif (brightness EQUAL 2)
        set(coeff 255)
    endif ()
    list(GET base_color 0 color_name)
    list(GET base_color 1 r)
    list(GET base_color 2 g)
    list(GET base_color 3 b)
    list(GET base_color 4 a)
    math(EXPR r "${r} * ${coeff} / 255")
    math(EXPR g "${g} * ${coeff} / 255")
    math(EXPR b "${b} * ${coeff} / 255")
    set("${new_color}" "${color_name}_${coeff};${r};${g};${b};${a}" PARENT_SCOPE)
endfunction()

function(parse_color_map FILENAME OUTPUT)
    file(STRINGS "${FILENAME}" content)
    set(lineno 0)
    set(result "")
    foreach(line ${content})
        math(EXPR lineno "${lineno} + 1" OUTPUT_FORMAT DECIMAL)
        string(STRIP ${line} line_)
        # is this an empty line?
        if (NOT line_)
            continue ()
        endif ()
        # is this a comment?
        if (line_ MATCHES "^#.*$")
            continue ()
        endif ()
        # parse base colors
        separate_arguments(parts UNIX_COMMAND "${line_}")
        list(LENGTH parts parts_len)
        if (parts_len LESS 2)
            message(FATAL_ERROR "Invalid content on line ${lineno}: ${line}")
        endif ()
        list(GET parts 0 rgb)
        math(EXPR rgb "${rgb}")
        list(GET parts 1 color_id)
        math(EXPR color_id "${color_id}")
        if (parts_len EQUAL 3)
            list(GET parts 2 color_name)
        else ()
            set(color_name "base id ${color_id}")
        endif ()
        # extract R G B components
        math(EXPR c_r "${rgb} >> 16")
        math(EXPR c_g "${rgb} >> 8 & 0x000000ff")
        math(EXPR c_b "${rgb} & 0x000000ff")
        if (color_id EQUAL 0)
            set(c_a 0)
        else ()
            math(EXPR c_a "0xFF")
        endif()

        foreach(brightness 0 1 2 3)
            derive_brightness("${color_name};${c_r};${c_g};${c_b};${c_a}" "${brightness}" new_color)
            list(APPEND result "${new_color}")
        endforeach()
    endforeach()
    set("${OUTPUT}" "${result}" PARENT_SCOPE)
endfunction()

function(generate_c_header input_name c_identifier colors output)
    list(LENGTH colors colors_len)
    math(EXPR height "${colors_len} / 5 / 4")
    math(EXPR array_size "${colors_len} / 5 * 4")
    set(accumulator
"# 0 \"${input_name}\"\n\
const size_t ${c_identifier}_HEIGHT = ${height};\n")
    set(accumulator
"${accumulator}# 1 \"${input_name}\"\n\
const uint8_t ${c_identifier}_DATA[${array_size}] = { ")
    foreach(start RANGE 1 "${colors_len}" 5)
        list(SUBLIST colors "${start}" 4 rgba)
        list(JOIN rgba ", " rgba)
        set(accumulator "${accumulator}${rgba}, ")
    endforeach()
    set(accumulator
"${accumulator}};\n\
// safe-guards\n\
static_assert(${c_identifier}_HEIGHT > 0, \"${c_identifier}_HEIGHT must be greater than zero!\");\n")

    set("${output}" "\n${accumulator}" PARENT_SCOPE)
endfunction()

function(generate_gimp_color_book colors prefix output)
    set(preamble "GIMP Palette\nName: ${prefix}\nColumns: 4\n#\n")
    set(accumulator "")
    list(LENGTH colors colors_len)
    # skip the first 4 colors (4 * 5 elements)
    foreach(start RANGE 20 "${colors_len}" 5)
        if (start EQUAL colors_len)
            break ()
        endif ()
        list(GET colors "${start}" color_name)
        math(EXPR color_start "${start} + 1")
        list(SUBLIST colors "${color_start}" 3 rgb)
        list(JOIN rgb "\t" rgb)
        set(accumulator "${accumulator}${rgb}\t#${color_name}\n")
    endforeach()
    set("${output}" "${preamble}${accumulator}" PARENT_SCOPE)
endfunction()