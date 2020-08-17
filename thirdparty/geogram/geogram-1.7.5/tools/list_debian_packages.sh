# Lists the Debian packages used by an executable
# Usage: list_debian_package.sh executable_name

if [ -z $1 ]
then
    echo "Usage: list_debian_packages.sh executable_name"
    exit -1
fi


# Get all the shared objects used by the executable.
LIBS=`ldd $1 | awk '{print $3}'`

# For each shared object, find the corresponding Debian package
PACKAGES=`dpkg -S $LIBS | grep amd64 | egrep -v nvidia | awk '{print $1}' | sed 's|\:$||'`

# Get the version of each Debian package
for PACKAGE in $PACKAGES
do
    PACKAGE_VERSION=`dpkg -s $PACKAGE | grep 'Version:' | awk '{ print $2 }'`
    PACKAGES_WITH_VERSIONS="$PACKAGES_WITH_VERSIONS, $PACKAGE (>= $PACKAGE_VERSION)" 
done

# Remove the leading ", "
echo $PACKAGES_WITH_VERSIONS | sed -e 's|^, ||'



            
