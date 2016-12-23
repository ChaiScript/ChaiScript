from conans import ConanFile
from conans.tools import download, untargz
import os

class Settings:
    username = os.getenv('CONAN_CHAISCRIPT_USERNAME', 'Manu343726')
    channel  = os.getenv('CONAN_CHAISCRIPT_CHANNEL', 'testing')
    version  = os.getenv('CONAN_CHAISCRIPT_VERSION', '5.8.5')

class ChaiScript(ConanFile):
    settings = 'os'
    name = 'ChaiScript'
    url = 'http://chaiscript.com/'
    license = 'BSD'
    username = Settings.username
    channel = Settings.channel
    version = Settings.version
    exports = '*.hpp', '*.chai'
    generators = 'cmake'

    def source(self):
        name = '{}-{}'.format(self.name, self.version)
        tar = name + '.tar.gz'
        url = 'https://github.com/ChaiScript/ChaiScript/archive/v{}.tar.gz'.format(self.version)
        download(url, tar)
        untargz(tar)

    def package(self):
        includedir = os.path.join('include', 'chaiscript')
        src_includedir = os.path.join('{}-{}'.format(self.name, self.version), includedir)
        self.copy('*.hpp', src=src_includedir, dst=includedir)
        self.copy('*.chai', src=src_includedir, dst=includedir)

    def package_info(self):
        if self.settings.os == 'Linux':
            self.cpp_info.libs = ['dl', 'pthread']
