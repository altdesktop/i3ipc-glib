from ipctest import IpcTest
import pytest


@pytest.mark.skip(reason='TODO')
class TestRestart(IpcTest):
    def test_auto_reconnect(self, i3):
        i3.auto_reconnect = True
        i3.command('restart')
        assert i3.command('nop')
