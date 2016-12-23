from conans import ConanFile, CMake
from conans.tools import download, untargz
import os

class Settings:
    username = os.getenv('CONAN_CHAISCRIPT_USERNAME', 'Manu343726')
    channel  = os.getenv('CONAN_CHAISCRIPT_CHANNEL', 'testing')
    version  = os.getenv('CONAN_CHAISCRIPT_VERSION', '5.8.5')

class ChaiScriptTest(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    requires = (
        'cmake-utils/0.0.0@Manu343726/testing',
        'ChaiScript/{}@{}/{}'.format(Settings.version, Settings.username, Settings.channel)
    )
    generators = 'cmake'

    def build(self):
        cmake = CMake(self.settings)
        self.run('cmake {} {}'.format(self.conanfile_directory, cmake.command_line))
        self.run('cmake --build . {}'.format(cmake.build_config))

    def test(self):
        self.run(os.path.join('.', 'bin', 'example'))

