project('gst-plugins-advent', 'c',
  version : '0.1.0',
  meson_version : '>= 0.36.0',
  default_options : [ 'warning_level=1',
                      'buildtype=debugoptimized' ])

gst_version = meson.project_version()
version_arr = gst_version.split('.')
gst_version_major = version_arr[0]
gst_version_minor = version_arr[1]
gst_version_micro = version_arr[2]
if version_arr.length() == 4
  gst_version_nano = version_arr[3]
else
  gst_version_nano = 0
endif

src = ['src/gstdecodetimebin.c']

gst_dep = dependency('gstreamer-1.0', version : '>1.0')

cdata = configuration_data()
cdata.set_quoted('PACKAGE', meson.project_name())
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set('VERSION', '"@0@"'.format(gst_version))

configure_file(output : 'config.h', configuration : cdata)

gstadvent = library('gstadvent', src,
    c_args : ['-DHAVE_CONFIG_H'],
    dependencies : gst_dep)
