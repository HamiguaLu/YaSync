
MyAddinps.dll: dlldata.obj MyAddin_p.obj MyAddin_i.obj
	link /dll /out:MyAddinps.dll /def:MyAddinps.def /entry:DllMain dlldata.obj MyAddin_p.obj MyAddin_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del MyAddinps.dll
	@del MyAddinps.lib
	@del MyAddinps.exp
	@del dlldata.obj
	@del MyAddin_p.obj
	@del MyAddin_i.obj
