package View;

import Controller.Controller;
import javax.swing.*;
import java.awt.*;

/**
 * @author Nezar, Alban on 2022-12-20.
 * @project Project_group_2
 */
public class PanelSouth extends JPanel {
    private Controller controller;
    private int width;
    private int height;
    BorderLayout layout;
    private PanelNode5_Node6 panelNode5_node6;
    private PanelNode7_Node8 panelNode7_node8;
    public PanelSouth(Controller controller, int width, int height) {
        this.controller = controller;
        this.width = width;
        this.height = height;
        setupPanel();
    }

    private void setupPanel() {
        layout = new BorderLayout();
        setLayout(layout);

        panelNode5_node6 = new PanelNode5_Node6(controller,6* width/10, 6*height/10, 6);
        add(panelNode5_node6,BorderLayout.WEST);
        panelNode7_node8 = new PanelNode7_Node8(controller,6* width/10, 6*height/10, 6);
        add(panelNode7_node8,BorderLayout.EAST);
    }

    public void updateNode5(String[] node5_info) {
        panelNode5_node6.updateNode5(node5_info);
    }

    public void updateNode6(String[] node6_info) {
        panelNode5_node6.updateNode6(node6_info);
    }

    public void updateNode7(String[] node7_info) {
        panelNode7_node8.updateNode7(node7_info);
    }

    public void updateNode8(String[] node8_info) {
        panelNode7_node8.updateNode8(node8_info);
    }

}
