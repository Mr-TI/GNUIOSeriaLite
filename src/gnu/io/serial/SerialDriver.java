package gnu.io.serial;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class SerialDriver {

	public static native void setDebugEnabled(boolean enabled);

	private static native int _open(String portName, String options) throws IOException;

	private static native void _close(int fd);
	
	private static native int _available(int fd) throws IOException;
	
	private static native int _read(int fd, byte[] b, int off, int len) throws IOException;
	
	private static native int _read(int fd) throws IOException;
	
	private static native void _write(int fd, int b) throws IOException;
	
	private static native void _write(int fd, byte[] b, int off, int len) throws IOException;
	
	static {
		System.loadLibrary("gnuioserial");
	}

	private int fd = -1;

	public SerialDriver(String portName, String options) throws IOException {
		this.fd = _open(portName, options);
	}

	public void close() {
		_close(fd);
	}

	public InputStream getInputStream() throws IOException {
		return new InputStream() {
			
			@Override
			public int available() throws IOException {
				return _available(fd);
			}
			
			@Override
			public int read() throws IOException {
				return _read(fd);
			}
			
			@Override
			public int read(byte[] b, int off, int len) throws IOException {
				return _read(fd, b, off, len);
			}
		};
	}

	public OutputStream getOutputStream() throws IOException {
		return new OutputStream() {
			
			@Override
			public void write(int b) throws IOException {
				_write(fd, b);
			}
			
			@Override
			public void write(byte[] b, int off, int len) throws IOException {
				_write(fd, b, off, len);
			}
		};
	}
}
