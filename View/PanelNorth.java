package View;

import Controller.Controller;
import javax.swing.*;
import java.awt.*;

/**
 * @author Nezar, Alban on 2022-12-20.
 * @project Project_group_2
 */
public class PanelNorth extends JPanel {
    private Controller controller;
    private int width;
    private int height;
    BorderLayout layout;
    private PanelNode1_Node2 panelNode1_node2;
    private PanelNode3_Node4 panelNode3_node4;
    public PanelNorth(Controller controller, int width, int height) {
        this.controller = controller;
        this.width = width;
        this.height = height;
        setupPanel();
    }

    private void setupPanel() {
        layout = new BorderLayout();
        setLayout(layout);

        panelNode1_node2 = new PanelNode1_Node2(controller,6* width/10, 6*height/10, 6);
        add(panelNode1_node2,BorderLayout.WEST);
        panelNode3_node4 = new PanelNode3_Node4(controller,6* width/10, 6*height/10, 6);
        add(panelNode3_node4,BorderLayout.EAST);
    }

    public void updateNode1(String[] node1_info) {
        panelNode1_node2.updateNode1(node1_info);
    }

    public void updateNode2(String[] node2_info) {
        panelNode1_node2.updateNode2(node2_info);
    }

    public void updateNode3(String[] node3_info) {
        panelNode3_node4.updateNode3(node3_info);
    }

    public void updateNode4(String[] node4_info) {
        panelNode3_node4.updateNode4(node4_info);
    }
}
