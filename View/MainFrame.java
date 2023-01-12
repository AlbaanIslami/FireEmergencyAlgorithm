package View;

import Controller.Controller;
import javax.swing.*;

/**
 * @author Nezar, Alban on 2022-12-20.
 *  @project Project_group_2
 */
public class MainFrame extends JFrame{
    private int width = 1600;
    private int height= 600;

    Controller controller;
    MainPanel panel;
    public MainFrame(Controller controller){
        this.controller = controller;
        setupFrame();
    }
    public void setupFrame(){
        final int offsetX = width/5;
        final int offsetY = height/5;
        setSize(width,height);
        setTitle("NODES OUTPUT----PROGRAMED BY THE BEST GROUP NO 2 :) Nezar, Alban, Gabriella, Rasmus, Christian");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocation(offsetX,offsetY);

        panel = new MainPanel(controller,width,height);
        setContentPane(panel);
        setResizable(false);
        pack();
        setVisible(true);
    }

    public void updateNode1(String[] Node1_Info) {
        panel.panelNorth.updateNode1(Node1_Info);
    }
    public void updateNode2(String[] Node2_Info) {
        panel.panelNorth.updateNode2(Node2_Info);
    }

    public void updateNode3(String[] Node3_Info) {
        panel.panelNorth.updateNode3(Node3_Info);
    }

    public void updateNode4(String[] Node4_Info) {
        panel.panelNorth.updateNode4(Node4_Info);
    }

    public void updateNode5(String[] Node5_Info) {
        panel.panelSouth.updateNode5(Node5_Info);
    }

    public void updateNode6(String[] Node6_Info) {
        panel.panelSouth.updateNode6(Node6_Info);
    }

    public void updateNode7(String[] Node7_Info) {
        panel.panelSouth.updateNode7(Node7_Info);
    }

    public void updateNode8(String[] Node8_Info) {
        panel.panelSouth.updateNode8(Node8_Info);
    }
}
