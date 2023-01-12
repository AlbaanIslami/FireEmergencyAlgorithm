package View;

import Controller.Controller;
import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.border.CompoundBorder;
import java.awt.*;

/**
 * @author Nezar, Alban on 2022-12-20.
 * @project Project_group_2
 */
public class PanelNode1_Node2 extends JPanel {
    private JList<String> node1;
    private JList<String> node2;
    private Controller controller;

    public PanelNode1_Node2(Controller controller, int width, int height, int margin)
    {
        this.controller = controller;

        Border border = this.getBorder();
        Border emptyBorder = BorderFactory.createEmptyBorder(margin, margin, margin, margin);
        setBorder(new CompoundBorder(border, emptyBorder));

        Font font = new Font("New Times Roman", Font.PLAIN, 14);
        setBorder(BorderFactory.createTitledBorder("                                  NODE 1                                                                                NODE 2                                   "));

        node1 = new JList<>();
        node2 = new JList<>();

        node1.setFont(font);
        node2.setFont(font);

        JScrollPane s = new JScrollPane(node1);
        JScrollPane s1 = new JScrollPane(node2);

        s.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s.setPreferredSize(new Dimension(350-margin, height-2*margin));
        s1.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s1.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s1.setPreferredSize(new Dimension(350-margin, height-2*margin));

        node1.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        node2.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        add(s,BorderLayout.WEST);
        add(s1,BorderLayout.CENTER);
    }
    public void updateNode1(String[] node1_info)
    {
        node1.setListData(node1_info);
    }
    public void updateNode2(String[] node2_info) {
        {
            node2.setListData(node2_info);
        }
    }
}

