/*
    Author: <Nezar Sheikhi>
    Id: <ak9472>
    Study Program: <DT>
*/

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
public class PanelNode3_Node4 extends JPanel {
    private JList<String> node3;
    private JList<String> node4;
    private Controller controller;

    public PanelNode3_Node4(Controller controller, int width, int height, int margin) {
        this.controller = controller;

        Border border = this.getBorder();
        Border emptyBorder = BorderFactory.createEmptyBorder(margin, margin, margin, margin);
        setBorder(new CompoundBorder(border, emptyBorder));
        //data has type Object[]

        Font font = new Font("New Times Roman", Font.PLAIN, 14);

        setBorder(BorderFactory.createTitledBorder("                                  NODE 3                                                                                NODE 4                                   "));

        node3 = new JList<>();
        node4 = new JList<>();
        node3.setFont(font);
        node4.setFont(font);
        JScrollPane s = new JScrollPane(node3);
        JScrollPane s1 = new JScrollPane(node4);
        s.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s.setPreferredSize(new Dimension(350-margin, height-2*margin));
        s1.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        s1.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        s1.setPreferredSize(new Dimension(350-margin, height-2*margin));

        node3.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        node4.setSelectionMode(DefaultListSelectionModel.SINGLE_SELECTION);
        add(s,BorderLayout.WEST);
        add(s1,BorderLayout.CENTER);

    }

    public void updateNode3(String[] node3_info) {
        node3.setListData(node3_info);
    }

    public void updateNode4(String[] node4_info) {
        node4.setListData(node4_info);
    }
}