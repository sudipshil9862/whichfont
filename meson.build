project('whichfont', 'c',
  version : '0.1',
  default_options : ['warning_level=3']
)

fontconfig_dep = dependency('fontconfig')
glib_dep = dependency('glib-2.0')
pango_dep = dependency('pango')
pangoft2_dep = dependency('pangoft2')

executable('whichfont', 'whichfont.c',
  dependencies: [fontconfig_dep, glib_dep, pango_dep, pangoft2_dep],
  install: true,
)
