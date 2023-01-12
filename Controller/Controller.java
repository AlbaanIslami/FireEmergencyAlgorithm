package Controller;

import View.MainFrame;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;

/**
 * @author NezarAlban on 2022-12-20.
 * @project Project_group_2
 */

public class Controller {
    MainFrame view;
    public Controller() throws IOException {
        view = new MainFrame(this);
        server();

    }
    public void server(){
        try {
            ServerSocket serverSocket = new ServerSocket(3000);
            System.out.println("Server started on port 3000");

            while (true) {
                Socket clientSocket = null;
                try {
                    clientSocket = serverSocket.accept();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            assert clientSocket != null;
            System.out.println("Client connected: " + clientSocket.getInetAddress());
                new Thread(new ClientHandler(clientSocket)).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    class ClientHandler implements Runnable {
        private final Socket clientSocket;
        ArrayList<String> data = new ArrayList<>();

    public ClientHandler(Socket socket) {
            this.clientSocket = socket;
        }

        @Override
        public void run() {
            try {
                BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                String inputLine;
                while ((inputLine = in.readLine()) != null) {
                    if (!inputLine.equals("")) {
                        data.add(inputLine);
                        checkString();
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        private void checkString() {
            for (int i = 0; i < data.size(); i++) {
                switch (data.get(i)) {
                    case "NODE1" -> view.updateNode1(data.toArray(new String[0]));
                    case "NODE2" -> view.updateNode2(data.toArray(new String[0]));
                    case "NODE3" -> view.updateNode3(data.toArray(new String[0]));
                    case "NODE4" -> view.updateNode4(data.toArray(new String[0]));
                    case "NODE5" -> view.updateNode5(data.toArray(new String[0]));
                    case "NODE6" -> view.updateNode6(data.toArray(new String[0]));
                    case "NODE7" -> view.updateNode7(data.toArray(new String[0]));
                    case "NODE8" -> view.updateNode8(data.toArray(new String[0]));
                }
            }
        }
    }
}
