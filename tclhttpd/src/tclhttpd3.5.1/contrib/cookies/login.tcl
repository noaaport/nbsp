# ---------------------------------------------------------------------
#
# Author: Acacio Cruz
#
# Simple cookie handler for the login.tml form
#
# 2002/02/02 acruz	Creation date
# ---------------------------------------------------------------------

Direct_Url /login login

# ---------------------------------------------------------------------

proc login { {username none} {password none} } {
   global env

   set expire [clock format [expr {[clock seconds] + 3600}] \
                -format "%A, %d-%b-%Y %H:%M:%S GMT" -gmt 1]

   Doc_SetCookie -name username -value $username -expires $expire \
                 -path {/}  -domain {.yourdomain.com}

   set html [html::head "Login"]

   append html "Username: $username <BR>\n"
   append html "Password: $password <BR>\n"

   append html [html::footer]
   return $html
}
