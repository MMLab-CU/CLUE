# A script for installing relevant dependencies for building Parrots (on Travis)

case $(uname) in
  Linux)   # Linux (Ubuntu)

    # add additional repositories
    sudo add-apt-repository --yes ppa:kalakris/cmake
    if [ "$CXX" == "g++" ]; then
      sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test;
    fi

    # update apt database
    sudo apt-get update -qq -y

    # install new version of g++
    if [ "$CXX" == "g++" ]; then
      sudo apt-get install -qq g++-4.8;
      export CXX="g++-4.8";
    fi
    echo "CXX=${CXX}"

    # install cmake
    sudo apt-get install cmake

    # install gtest
    sudo apt-get install libgtest-dev
    cd /usr/src/gtest && sudo cmake . && sudo cmake --build . && sudo mv libg* /usr/local/lib/ ; cd -
    ;;

  Darwin)  # Mac OS X
    # update brew database
    brew update
    brew install cmake
    ;;

  *)
    echo "The install-deps.sh script only works with Linux or Mac OS X platforms, exiting ..."
    exit 1
    ;;
esac
