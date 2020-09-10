# Python script to download the latest Houdini production builds
#
# Note that this can now be replaced with this API:
# https://www.sidefx.com/docs/api/download/index.html
#
# Author: Dan Bailey

import mechanize
import sys
import re
import exceptions

# this argument is for the major.minor version of Houdini to download (such as 15.0, 15.5, 16.0)
version = sys.argv[1]
password = sys.argv[2]

if not re.match('[0-9][0-9]\.[0-9]$', version):
    raise IOError('Invalid Houdini Version "%s", expecting in the form "major.minor" such as "16.0"' % version)

br = mechanize.Browser()
br.set_handle_robots(False)

# login to sidefx.com as openvdb
br.open('https://www.sidefx.com/login/?next=/download/daily-builds')
br.select_form(nr=0)
br.form['username'] = 'openvdb'
br.form['password'] = password
br.submit()

# retrieve download id
br.open('https://www.sidefx.com/download/daily-builds/')

houid = -1

for link in br.links():
    if '/download/download-houdini' not in link.url:
        continue
    if link.text.startswith('houdini-%s' % version) and 'linux_x86_64' in link.text:
        response = br.follow_link(text=link.text, nr=0)
        url = response.geturl()
        houid = url.split('/download-houdini/')[-1]
        break

# download houdini tarball in 50MB chunks
url = 'https://www.sidefx.com/download/download-houdini/%sget/' % houid
response = br.open(url)
mb = 1024*1024
chunk = 50
size = 0
file = open('hou.tar.gz', 'wb')
for bytes in iter((lambda: response.read(chunk*mb)), ''):
    size += 50
    print 'Read: %sMB' % size
    file.write(bytes)
file.close()
