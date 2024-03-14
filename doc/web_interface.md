# NymphCast MediaServer - Web Interface #

This document describes the web interface provided by the NymphCast MediaServer (NCMS) and its functions.

## Dashboard ##

Upon opening the web interface in a supported browser, the dashboard is displayed. This provides status information about the local system, statistics on used and free disk space, configured media shares, etc.


## File Manager ##

The file manager can be accessed from the dashboard. It allows for the browsing of existing files in the configured folders, as well as the adding, editing (renaming, etc.), downloading & removal of files and folders. 

Additional shares can also be added here, and existing shares edited and removed.


## Authentication ##

Access to the web interface requires logging in with a password and username that is set in the NCMS configuration file, which can only be edited by either physical access to the host system, or by logging into it remotely via e.g. SSH.

Upon successful login a cookie is set that links the session with the system that was logged in from. This cookie is rendered invalid upon logout or after the time-out period has been reached.
