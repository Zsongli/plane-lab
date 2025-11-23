function embed_resources(input_dir, output_dir)
    os.mkdir(output_dir)
       
    local index_file = path.join(output_dir, "index.h")
    local index_out = io.open(index_file, "w")
    if not index_out then
        error("Failed to write " .. index_file)
    end

    index_out:write("// Auto-generated resource index\n\n")
    index_out:write("#pragma once\n\n")
    index_out:write("#define RESOURCE_DATA(index) (_resource_data[index])\n")
    index_out:write("#define RESOURCE_SIZE(index) (_resource_sizes[index])\n\n")
    
    local resource_data_array = "static const void* _resource_data[] = {\n"
    local resource_size_array = "static const size_t _resource_sizes[] = {\n"

    local files = os.matchfiles(input_dir .. "/*")
    for i, filepath in ipairs(files) do
        local filename = path.getname(filepath)
        local filename_snake_case = to_snake_case(filename)
        local outname = filename_snake_case .. ".h"
        local define_name = filename_snake_case:upper()
        local varname = define_name .. "_DATA"

        local outfile = path.join(output_dir, outname)
        print(string.format("Embedding resource %s -> %s", filename, outfile))
        embed_resource(filepath, outfile, varname)

        index_out:write(string.format("#include \"%s\"\n", outname))
        index_out:write(string.format("#define %s %d\n\n", define_name, i-1))
        resource_data_array = resource_data_array .. string.format("    %s,\n", varname)
        resource_size_array = resource_size_array .. string.format("    sizeof(%s),\n", varname)
    end
    resource_data_array = resource_data_array .. "};\n"
    resource_size_array = resource_size_array .. "};\n"
    index_out:write(resource_data_array)
    index_out:write("\n" .. resource_size_array)
    index_out:close()
end

function embed_resource(input_path, output_path, varname)
    local infile = io.open(input_path, "rb")
    if not infile then
        error("Failed to open " .. input_path)
    end
    local data = infile:read("*all")
    infile:close()

    local outfile = io.open(output_path, "w")
    if not outfile then
        error("Failed to write " .. output_path)
    end

    outfile:write("// Auto-generated resource file\n\n")
    outfile:write("#pragma once\n\n")
    outfile:write(string.format("static const unsigned char %s[] = {", varname))
    for i = 1, #data do
        if i % 12 == 1 then
            outfile:write("\n    ")
        end
        outfile:write(string.format("0x%02X,", data:byte(i)))
    end
    outfile:write("\n};\n")
    outfile:close()
end

function to_snake_case(filename)
    local name, ext = filename:match("^(.-)%.([^%.]+)$")
    
    if not name then
        name = filename
        ext = nil
    end

    name = name:gsub("[%s%-]+", "_")
    name = name:gsub("(%l)(%u)", "%1_%2")
    name = name:lower()
    name = name:gsub("_+", "_")
    name = name:gsub("^_+", ""):gsub("_+$", "")

    if ext then
        name = name .. "_" .. ext:lower()
    end

    return name
end
