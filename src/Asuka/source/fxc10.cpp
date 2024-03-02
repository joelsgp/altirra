//	Asuka - VirtualDub Build/Post-Mortem Utility
//	Copyright (C) 2005-2012 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdafx.h>
#include <vector>
#include <list>
#include <string>
#include <d3d11.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <vd2/system/refcount.h>
#include <vd2/system/error.h>
#include <vd2/system/file.h>
#include <vd2/system/filesys.h>
#include <vd2/system/hash.h>
#include <vd2/system/math.h>
#include <vd2/system/strutil.h>
#include <vd2/system/vdstl.h>

#pragma comment(lib, "d3dcompiler")

class FXC10IncludeHandler : public ID3DInclude {
public:
	FXC10IncludeHandler(const char *basePath) : mBasePath(VDTextAToW(basePath)) {}

    HRESULT STDMETHODCALLTYPE Open(D3D10_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
    HRESULT STDMETHODCALLTYPE Close(LPCVOID pData);

private:
	const VDStringW mBasePath;
};

HRESULT STDMETHODCALLTYPE FXC10IncludeHandler::Open(D3D10_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
	try {
		vdblock<char> buf;

		const VDStringW& path = VDFileResolvePath(mBasePath.c_str(), VDTextAToW(pFileName).c_str());

		VDFile f(path.c_str());

		uint32 s = (uint32)f.size();
		buf.resize(s);
		f.read(buf.data(), s);

		void *p = malloc(s);

		memcpy(p, buf.data(), s);

		*ppData = p;
		*pBytes = s;
	} catch(const MyError&) {
		return E_FAIL;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE FXC10IncludeHandler::Close(LPCVOID pData) {
	free((void *)pData);
	return S_OK;
}

///////////////////////////////////////////////////////////////////////////

void tool_fxc10(const vdfastvector<const char *>& args, const vdfastvector<const char *>& switches) {
	if (args.size() != 2) {
		puts("usage: asuka fxc10 source.fx target.cpp");
		exit(5);
	}

	const char *filename = args[0];

	printf("Asuka: Compiling effect file (D3D10): %s -> %s.\n", filename, args[1]);

	VDFile f(filename);
	FILE *fo = fopen(args[1], "w");
	if (!fo)
		printf("Asuka: Unable to open for write: %s\n", args[1]);

	try {
		fprintf(fo, "// Generated by Asuka from %s. DO NOT EDIT.", filename);

		uint32 len = VDClampToUint32(f.size());

		vdblock<char> buf(len);
		f.read(buf.data(), len);
		f.close();

		VDMemoryStream ms(buf.data(), len);
		VDTextStream ts(&ms);
		while(const char *line = ts.GetNextLine()) {
			line = strskipspace(line);

			if (line[0] != '/' || line[1] != '/')
				continue;

			line += 2;

			line = strskipspace(line);

			if (line[0] != '$' || line[1] != '$')
				continue;

			line += 2;

			if (strncmp(line, "export_shader", 13))
				continue;

			line += 13;

			if (!isspace((unsigned char)*line))
				continue;

			line = strskipspace(line);

			const char *const target_start = line;

			while(*line && !isspace((unsigned char)*line))
				++line;

			const char *const target_end = line;

			if (!isspace((unsigned char)*line))
				continue;

			line = strskipspace(line);

			const char *const function_start = line;
			while(*line && !isspace((unsigned char)*line))
				++line;

			const char *const function_end = line;

			if (!isspace((unsigned char)*line))
				continue;

			line = strskipspace(line);

			const char *const symbol_start = line;
			while(*line && !isspace((unsigned char)*line))
				++line;

			const char *const symbol_end = line;
			bool multitarget = false;

			VDStringA target_name(target_start, target_end);
			VDStringA function_name(function_start, function_end);
			VDStringA symbol_name(symbol_start, symbol_end);

			printf("Asuka: compile %s %s() -> %s\n", target_name.c_str(), function_name.c_str(), symbol_name.c_str());

			vdvector<VDStringA> targets;
			if (!target_name.empty() && target_name[0] == '[') {
				VDStringRefA targets_parse(target_name.c_str() + 1);
				VDStringRefA target_token;

				for(;;) {
					if (!targets_parse.split(',', target_token)) {
						targets_parse.split(']', target_token);
						targets.push_back(target_token);
						break;
					}

					targets.push_back(target_token);
				}

				multitarget = true;
			} else {
				targets.push_back(target_name);
			}

			vdfastvector<uint8> shaderdata;
			vdfastvector<uint32> shadermetadata;

			fputs("\n\n", fo);

			uint32 target_count = 0;
			while(!targets.empty()) {
				VDStringA compile_target = targets.back();

				targets.pop_back();

				bool isd3d9 = false;
				bool isd3d10 = false;

				if (compile_target == "vs_1_1"
					|| compile_target == "vs_2_0"
					|| compile_target == "vs_3_0"
					|| compile_target == "ps_1_1"
					|| compile_target == "ps_1_2"
					|| compile_target == "ps_1_3"
					|| compile_target == "ps_2_0"
					|| compile_target == "ps_2_a"
					|| compile_target == "ps_2_b"
					|| compile_target == "ps_3_0")
				{
					isd3d9 = true;
				}
				else if (compile_target == "vs_4_0_level_9_1"
					|| compile_target == "vs_4_0_level_9_3"
					|| compile_target == "ps_4_0_level_9_1"
					|| compile_target == "ps_4_0_level_9_3")
				{
					isd3d10 = true;
				}

				const D3D_SHADER_MACRO macros[]={
					{ "PROFILE_D3D9", isd3d9 ? "1" : "0" },
					{ "PROFILE_D3D10", isd3d10 ? "1" : "0" },
					{ NULL, NULL },
				};

				FXC10IncludeHandler includeHandler(VDFileSplitPathLeft(VDStringA(filename)).c_str());

				vdrefptr<ID3D10Blob> shader;
				vdrefptr<ID3D10Blob> errors;
				HRESULT hr = D3DCompile(buf.data(), len, filename, macros, &includeHandler, function_name.c_str(), compile_target.c_str(), 0, 0, ~shader, ~errors);

				if (FAILED(hr)) {
					printf("Effect compilation failed for \"%s\" with target %s (hr=%08x)\n", filename, compile_target.c_str(), (unsigned)hr);

					if (errors)
						puts((const char *)errors->GetBufferPointer());

					shader.clear();
					errors.clear();
					fclose(fo);
					remove(args[1]);
					exit(10);
				}

				const uint8 *compile_data = (const uint8 *)shader->GetBufferPointer();
				const uint32 compile_len = shader->GetBufferSize();

				vdrefptr<ID3D10Blob> disasm;
				hr = D3DDisassemble(compile_data, compile_len, 0, NULL, ~disasm);
				if (SUCCEEDED(hr)) {
					VDMemoryStream ms(disasm->GetBufferPointer(), disasm->GetBufferSize());
					VDTextStream ts(&ms);

					fprintf(fo, "/* -- %s --\n", compile_target.c_str());

					while(const char *line = ts.GetNextLine()) {
						fprintf(fo, "\t%s\n", line);
					}

					fputs("*/\n", fo);
				}

				disasm.clear();

				shadermetadata.push_back(VDHashString32I(compile_target.c_str()));
				shadermetadata.push_back(shaderdata.size());
				shadermetadata.push_back(compile_len);
				++target_count;

				shaderdata.insert(shaderdata.end(), compile_data, compile_data + compile_len);
			}

			if (multitarget) {
				shadermetadata.push_back(0);

				uint32 offset = shadermetadata.size() * 4;

				for(uint32 i=0; i<target_count; ++i)
					shadermetadata[i*3 + 1] += offset;

				const uint8 *metastart = (const uint8 *)shadermetadata.data();
				shaderdata.insert(shaderdata.begin(), metastart, metastart + shadermetadata.size() * 4);
			}

			const uint8 *data = shaderdata.data();
			uint32 data_len = shaderdata.size();

			fprintf(fo, "static const uint8 %s[] = {\n", symbol_name.c_str());

			while(data_len) {
				putc('\t', fo);

				uint32 tc = data_len > 16 ? 16 : data_len;

				for(uint32 i=0; i<tc; ++i)
					fprintf(fo, "0x%02x,", *data++);

				data_len -= tc;

				putc('\n', fo);
			}

			fprintf(fo, "};\n");
		}
	} catch(const MyError&) {
		fclose(fo);
		remove(args[1]);
		throw;
	}

	fclose(fo);

	printf("Asuka: Compilation was successful.\n");
}
