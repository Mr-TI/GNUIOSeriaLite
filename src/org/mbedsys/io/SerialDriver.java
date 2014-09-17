package org.mbedsys.io;

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
	
	private static native void _flush(int fd) throws IOException;
	
	static {
		System.loadLibrary("mserialite");
	}

	private int fd = -1;
	private InputStream instream = new InputStream() {
		
		@Override
		public synchronized int available() throws IOException {
			return _available(fd);
		}
		
		@Override
		public synchronized int read() throws IOException {
			return _read(fd);
		}
		
		@Override
		public synchronized int read(byte[] b, int off, int len) throws IOException {
			return _read(fd, b, off, len);
		}
	};
	private OutputStream outstream = new OutputStream() {
		
		@Override
		public synchronized void write(int b) throws IOException {
			_write(fd, b);
		}
		
		@Override
		public synchronized void write(byte[] b, int off, int len) throws IOException {
			_write(fd, b, off, len);
		}
		
		@Override
		public synchronized void flush() throws IOException {
			_flush(fd);
		};
	};

	public SerialDriver(String portName, String options) throws IOException {
		this.fd = _open(portName, options);
	}

	public void close() {
		_close(fd);
	}

	public InputStream getInputStream() throws IOException {
		return instream;
	}

	public OutputStream getOutputStream() throws IOException {
		return outstream;
	}
}
