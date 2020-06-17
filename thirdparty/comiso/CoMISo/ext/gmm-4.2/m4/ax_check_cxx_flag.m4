dnl based on http://www.gnu.org/software/ac-archive/htmldoc/ac_check_cc_opt.html
dnl from Guido Draheim <guidod@gmx.de>
AC_DEFUN([AC_CHECK_CXX_FLAG],
[AC_MSG_CHECKING([whether ${CXX} accepts $1])

echo 'int main(){}' > conftest.c
if test -z "`${CXX} $1 -o conftest conftest.c 2>&1`"; then
  $2="${$2} $1"
  echo "yes"
else
  echo "no"
fi
dnl echo "$2=${$2}"
rm -f conftest*
])

