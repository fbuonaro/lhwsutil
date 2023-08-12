componentName="lhwsutil"

###############################################################################
# BOILERPLATE START
###############################################################################
# TILT_GIT_RESOURCE_CHECKOUT_DIR can be used set the directory where dependent projects
LHBUILD_MODULES_DIR_ENV_NAME= "TILT_GIT_RESOURCE_CHECKOUT_DIR"
if not os.getenv( LHBUILD_MODULES_DIR_ENV_NAME, default = None ):
    os.putenv( LHBUILD_MODULES_DIR_ENV_NAME, os.path.abspath( "./modules" ) )
gitDir = os.getenv( LHBUILD_MODULES_DIR_ENV_NAME, default = None )
if not gitDir:
    fail( "failed to set modules checkout dir" )
load( "ext://git_resource", "git_checkout" )
git_checkout( "git@github.com:fbuonaro/lhscriptutil.git#release/v/1.3.0" )
commonTilt = load_dynamic( os.path.join( gitDir, "lhscriptutil", "tilt", "common", "Tiltfile") )
lhQuickBuildImage = commonTilt[ "lhQuickBuildImage" ]
lhQuickBuildImageNoK8 = commonTilt[ "lhQuickBuildImageNoK8" ]
trySetTopLevelComponent = commonTilt[ "trySetTopLevelComponent" ]
trySetTopLevelComponent(componentName)
lhscriptutilTilt = load_dynamic( os.path.join( gitDir, "lhscriptutil", "Tiltfile") )
cfg = lhscriptutilTilt[ "cfg" ]
###############################################################################
# <<< BOILERPLATE END
###############################################################################


lhGitCheckoutAndLoad( "git@github.com:fbuonaro/lhmiscutil.git#tilt", gitDir )

lhGitCheckoutAndLoad( "git@github.com:fbuonaro/lhsslutil.git#tilt", gitDir )

lhQuickBuildImageNoK8( "libjwt-lhdist", cfg, dockerfile="Dockerfiles/Dockerfile.libjwt-lhdist" )

lhQuickBuildImageNoK8( "lhwsutil", cfg, dockerfile="Dockerfiles/Dockerfile.lhwsutil" )
