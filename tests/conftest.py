import pktpy
import pytest


@pytest.fixture(scope="session", autouse=True)
def vm_lifecycle():
    pktpy.initialize()
    yield
    pktpy.finalize()
